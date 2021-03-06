#include <iostream>
#include <set>

#include "tensorflow_graph_tools.hpp"
#include "GraphManager.hpp"
#include "grid_tools.hpp"

#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/util/command_line_flags.h"
#include "tensorflow/core/framework/tensor_util.h"

int main(int argc, char* argv[])
{
    std::string graph = "graph.pb";
    std::string input_layer = "input_layer";
    std::string output_layer = "output_layer";
    std::string gradient_layer = "gradient_layer_tmp_placeholder";
    std::string granularity_str = "granularity";
    std::string verification_radius = "verification radius";
    std::string initial_activation = "initial_activation.pb";
    std::string root_dir = ".";
    std::string class_averages = "class_averages";
    std::string label_proto = "label_proto";
    std::string label_layer = "label_layer_placeholder";
    std::string fgsm_balance_factor_opt = "1.0";
    std::string enforce_domain_str = "true";
    std::string domain_range_min_str = "0.0";
    std::string domain_range_max_str = "1.0";
    std::string modified_fgsm_dim_selection = "largest_first";
    std::string num_abstractions_str = "1000";
    std::string abstraction_strategy_str = "modifiedFGSM";

    std::vector<tensorflow::Flag> flag_list = {
        tensorflow::Flag("graph", &graph, "path to protobuf graph to be executed"),
        tensorflow::Flag("input_layer", &input_layer, "name of input layer"),
        tensorflow::Flag("output_layer", &output_layer, "name of output layer"),
        tensorflow::Flag("gradient_layer", &gradient_layer, "name of the gradient layer (optional - used for FGSM)"),
        tensorflow::Flag("granularity", &granularity_str, "use this option is all dimensions share a discrete range"),
        tensorflow::Flag("verification_radius", &verification_radius, "'radius' of hyperrectangle within which safety is to be verified"),
        tensorflow::Flag("initial_activation", &initial_activation, "initial tested activation"),
        tensorflow::Flag("root_dir", &root_dir, "root_dir"),
        tensorflow::Flag("class_averages", &class_averages, "the class averages of the training data (optional - used for FGSM)"),
        tensorflow::Flag("label_proto", &label_proto, "protocol buffer of label image corresponding with initial activation"),
        tensorflow::Flag("label_layer", &label_layer, "name of label layer"),
        tensorflow::Flag("enforce_domain", &enforce_domain_str, "enforce the domain range (true, false)"),
        tensorflow::Flag("abstraction_strategy", &abstraction_strategy_str, "abstraction strategy (modifiedFGSM, R+FGSM"),
        tensorflow::Flag("domain_range_min", &domain_range_min_str, "lower bound on domain range (default = 0.0)"),
        tensorflow::Flag("domain_range_max", &domain_range_max_str, "upper bound on domain range (default = 1.0)"),
        tensorflow::Flag("modified_fgsm_dim_selection", &modified_fgsm_dim_selection, "dimension selection strategy to use (gradient_based, intellifeature, largest_first - default)"),
        tensorflow::Flag("num_abstractions", &num_abstractions_str, "number of abstractions to generate (adversarial examples)"),
        tensorflow::Flag("fgsm_balance_factor", &fgsm_balance_factor_opt, "Balance factor for modified FGSM algorithm (ratio dimensions fgsm/random)")
    };

    std::string usage = tensorflow::Flags::Usage(argv[0], flag_list);
    const bool parse_result = tensorflow::Flags::Parse(&argc, argv, flag_list);
    if (!parse_result) 
    {
        LOG(ERROR) << usage;
        return -1;
    }

    tensorflow::port::InitMain(argv[0], &argc, &argv);
    if (argc > 1) {
        LOG(ERROR) << "Unknown argument " << argv[1] << "\n" << usage;
        return -1;
    }

    const auto enforce_domain = enforce_domain_str == "true";
    if(gradient_layer == "gradient_layer_tmp_placeholder")
    {
        LOG(ERROR) << "must provide gradient layer\n";
        return -1;
    }
    if(granularity_str == "granularity")
    {
        LOG(ERROR) << "must provided granularity\n";
        return -1;
    }
    if(verification_radius == "verification radius")
    {
        LOG(ERROR) << "must provide verification radius";
        return -1;
    }
    auto granularity_val = std::atof(granularity_str.c_str());
    auto radius = std::atof(verification_radius.c_str());
    auto fgsm_balance_factor = std::atof(fgsm_balance_factor_opt.c_str());
    auto domain_range_min = std::atof(domain_range_min_str.c_str());
    auto domain_range_max = std::atof(domain_range_max_str.c_str());
    auto num_abstractions = std::atoi(num_abstractions_str.c_str());
    auto graph_path = tensorflow::io::JoinPath(root_dir, graph);
    GraphManager gm(graph_path);
    if(!gm.ok())
    {
        LOG(ERROR) << "Could not initialize graph manager at: "
            << graph_path;
        return -1;
    }

    auto initial_activation_path = 
        tensorflow::io::JoinPath(root_dir, initial_activation);
    auto init_act_tensor_status_pair = 
        GraphManager::ReadBinaryTensorProto(initial_activation_path);
    if(!init_act_tensor_status_pair.first)
    {
        LOG(ERROR) 
            << "Could not read initial activation protobuf: " 
            << initial_activation_path;
        return -1;
    }

    auto init_act_tensor = init_act_tensor_status_pair.second;
    auto init_act_point = graph_tool::tensorToPoint(
            init_act_tensor);

    std::vector<tensorflow::int64> batch_input_shape(init_act_tensor.dims());

    for(auto i = 0u; i < batch_input_shape.size(); ++i)
        batch_input_shape[i] = init_act_tensor.dim_size(i);

    auto logits_init_activation = 
        gm.feedThroughModel(
                std::bind(graph_tool::makeFeedDict,
                    input_layer,
                    init_act_point,
                    batch_input_shape),
                &graph_tool::parseGraphOutToVector,
                {output_layer});
    if(!gm.ok())
    {
        LOG(ERROR) << "Error while feeding through model";
        exit(1);
    }

    unsigned orig_class = 
        graph_tool::getClassOfClassificationVector(logits_init_activation);

    std::cout << "Original class: " << orig_class << "\n";

    std::vector<long double> granularity(init_act_point.size(), 
            granularity_val);

    std::cout << "Granularity: " << granularity_val << "\n";
    std::cout << "Input shape: (";
    for(auto&& elem : batch_input_shape)
        std::cout << elem << " ";
    std::cout << ")\n";
    std::cout << "Verification radius: " << radius << "\n";
    std::cout << "Root Directory: " << root_dir << "\n";

    grid::dimension_selection_strategy_t dimension_selection_strategy = 
        grid::largestDimFirst;
    if(modified_fgsm_dim_selection == "largest_first")
        std::cout << "Using largest first dimension selection strategy\n";


    auto hasAveragesProto = class_averages != "class_averages";
    auto hasLabelProto = label_proto != "label_proto";
    auto hasLabelLayer = label_layer != "label_layer_placeholder";

    if(!hasLabelProto || !hasLabelLayer)
    {
        LOG(ERROR) << "Need gradient layer, label proto, and label layer";
        return -1;
    }

    if(modified_fgsm_dim_selection == "random")
    {
        std::cout << "Using random dimension selection heuristic\n";
        dimension_selection_strategy = 
            grid::randomDimSelection;
    }

    if(hasAveragesProto && modified_fgsm_dim_selection == "intellifeature")
    {
        std::cout << "Using Intellifeature\n";
        std::string class_averages_path = 
            tensorflow::io::JoinPath(root_dir, class_averages);
        auto class_averages_proto_pair = 
            GraphManager::ReadBinaryTensorProto(class_averages_path);
        if(!class_averages_proto_pair.first)
        {
            LOG(ERROR) << "Unable to read class averages proto";
            return -1;
        }
        auto averages = graph_tool::tensorToPoints(
                class_averages_proto_pair.second);
        dimension_selection_strategy = 
            grid::IntellifeatureDimSelection(
                averages,
                &grid::l2norm,
                orig_class);
    }

    auto label_tensor_path = 
        tensorflow::io::JoinPath(root_dir, label_proto);
    auto label_tensor_pair =
        GraphManager::ReadBinaryTensorProto(label_tensor_path);
    if(!label_tensor_pair.first)
    {
        LOG(ERROR) << "Unable to read label proto";
        exit(1);
    }
    auto label_tensor = label_tensor_pair.second;

    std::cout << "FGSM Balance Factor: " << fgsm_balance_factor
        << "\n";
    auto grad_func = 
            [&,label_tensor_copy = label_tensor]
            (grid::point const& p) -> grid::point
            {
                auto createGradientFeedDict = 
                [&]() -> graph_tool::feed_dict_type_t
                {
                    auto p_tensor = 
                    graph_tool::pointToTensor(p, batch_input_shape);
                    return {{input_layer, p_tensor},
                        {label_layer, label_tensor_copy}};
                };
                auto retVal = gm.feedThroughModel(
                        createGradientFeedDict,
                        &graph_tool::parseGraphOutToVector,
                        {gradient_layer});
                if(!gm.ok())
                    LOG(ERROR) << "Error with model";
                return retVal;
            };

    if(modified_fgsm_dim_selection == "gradient_based")
    {
        std::cout << "Using gradient_based dimension selection\n";
        dimension_selection_strategy = 
            grid::GradientBasedDimensionSelection(grad_func);
    }

    grid::region_abstraction_strategy_t abstraction_strategy = 
        grid::ModifiedFGSMWithFallbackRegionAbstraction(
                num_abstractions,
                grad_func,
                dimension_selection_strategy,
                grid::RandomPointRegionAbstraction(1u),
                granularity,
                fgsm_balance_factor);

    if(abstraction_strategy_str == "R+FGSM")
    {
        abstraction_strategy = grid::RPlusFGSMAbstraction(
                num_abstractions,
                grad_func,
                granularity,
                0.1);
    }

    grid::region orig_region(init_act_point.size());
    for(auto i = 0u; i < orig_region.size(); ++i)
    {
        orig_region[i].first = 
            init_act_point[i] - radius;
        orig_region[i].second = 
            init_act_point[i] + radius;
    }

    grid::region domain_range(orig_region.size());
    for(auto i = 0u; i < domain_range.size(); ++i)
    {
        domain_range[i].first = domain_range_min;
        domain_range[i].second = domain_range_max;
    }

    orig_region = grid::snapToDomainRange(orig_region, domain_range);

    auto abstractions = abstraction_strategy(orig_region);
    std::set<grid::point> unique_abstractions;
    for(auto&& pt : abstractions)
    {
        auto pt_to_add = grid::enforceSnapDiscreteGrid(
            pt, init_act_point, granularity);
        if(enforce_domain)
        {
            pt_to_add = grid::snapToDomainRange(
                    pt_to_add, domain_range);
        }
        unique_abstractions.insert(pt_to_add);
    }
    std::cout << "Unique Abstractions: " 
        << unique_abstractions.size() << "\n";

    auto pointIsSafe = [&](grid::point const& p)
            {
                auto logits_out = gm.feedThroughModel(
                        std::bind(graph_tool::makeFeedDict, 
                            input_layer, p, batch_input_shape),
                        &graph_tool::parseGraphOutToVector,
                        {output_layer});
                if(!gm.ok())
                    LOG(ERROR) << "GM Error in pointIsSafe";
                auto class_out = 
                    graph_tool::getClassOfClassificationVector(
                            logits_out);
                return class_out == orig_class;
            };

    auto numAdvExamples = 0u;
    for(auto&& pt : unique_abstractions)
    {
        if(pointIsSafe(pt))
            continue;
        ++numAdvExamples;
    }
    std::cout << "Number Adversarial Examples: "
        << numAdvExamples << "\n";
    auto success_rate = (double)numAdvExamples / (double)unique_abstractions.size() * 100.0;
    std::cout << "Success Rate: " << success_rate << "\n";

    return 0;
}

