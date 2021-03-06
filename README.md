# ARFramework

## Installation instructions

VERAPAK uses docker to isolate build dependencies. Follow the instructions [here](https://docs.docker.com/get-docker/) to install docker on your system. After that, building is as simple as running the following command in this directory:
```
docker build -t <image_name>:<tag> .
```
This command takes about 45 minutes to run so go get a snack. :)

The executable is added to the path of the docker container and is called `ARFramework_main`

### cifar-10 example command
bazel-bin/tensorflow/ARFramework/ARFramework_main --root_dir=/home/jsmith/tensorflow/tensorflow/ARFramework/cifar10 --initial_activation=cifar_100.pb --granularity=0.00390625 --input_layer="input_layer_x" --output_layer="probabilities_out" --graph="cifar_gradient.pb" --verification_radius=0.2 --num_threads=10 --output_dir=/home/jsmith/output/cifar10 --num_abstractions=10 --label_layer="label_layer_y" --gradient_layer="gradient_out" --label_proto=cifar_100_label.pb --class_averages=cifar_10_averages.pb --fgsm_balance_factor=1.0 --modified_fgsm_dim_selection="intellifeature" --refinement_dim_selection="gradient_based"

### mnist example command
bazel-bin/tensorflow/ARFramework/ARFramework_main --root_dir=/home/jsmith/tensorflow/tensorflow/ARFramework/mnist --initial_activation=mnist_200.pb --granularity=0.00390625 --input_layer="x_input" --output_layer="probabilities_out" --graph="mnist_gradient.pb" --verification_radius=0.4 --num_threads=10 --output_dir=/home/jsmith/output/mnist --num_abstractions=10 --label_layer="y_label" --gradient_layer="gradient_out" --label_proto=mnist_200_label.pb --class_averages=mnist_averages.pb --fgsm_balance_factor=1.0 --modified_fgsm_dim_selection="intellifeature" --refinement_dim_selection="gradient_based"

### GTSRB example command
bazel-bin/tensorflow/ARFramework/ARFramework_main --root_dir=/home/jsmith/tensorflow/tensorflow/ARFramework/gtsrb --initial_activation=gtsrb_1200.pb --granularity=0.00390625 --input_layer="input_layer_x" --output_layer="probabilities_out" --graph="gtsrb_gradient.pb" --verification_radius=0.4 --num_threads=10 --output_dir=/home/jsmith/output/gtsrb --num_abstractions=10 --label_layer="label_layer_y" --gradient_layer="gradient_out" --label_proto=gtsrb_1200_label.pb --class_averages=gtsrb_averages.pb --fgsm_balance_factor=1.0 --modified_fgsm_dim_selection="intellifeature" --refinement_dim_selection="largest_first"

### FGSM Test example command
#### MNIST
bazel-bin/tensorflow/ARFramework/ARFramework_FGSM_test --graph="mnist_gradient.pb" --root_dir=/home/jsmith/tensorflow/tensorflow/ARFramework/mnist --initial_activation=mnist_200.pb --input_layer="x_input" --output_layer="probabilities_out" --gradient_layer="gradient_out" --granularity=0.00390625 --verification_radius=0.4 --class_averages=mnist_averages.pb --label_proto=mnist_200_label.pb --label_layer="y_label" --enforce_domain=true --domain_range_min=0.0 --domain_range_max=1.0 --fgsm_balance_factor=0.6 --modified_fgsm_dim_selection="gradient_based" --num_abstractions=1000

#### CIFAR-10
bazel-bin/tensorflow/ARFramework/ARFramework_FGSM_test --graph="cifar_gradient.pb" --root_dir=/home/jsmith/tensorflow/tensorflow/ARFramework/cifar10 --initial_activation=cifar_200.pb --input_layer="input_layer_x" --output_layer="probabilities_out" --gradient_layer="gradient_out" --granularity=0.00390625 --verification_radius=0.4 --class_averages=cifar_10_averages.pb --label_proto=cifar_200_label.pb --label_layer="label_layer_y" --enforce_domain=true --domain_range_min=0.0 --domain_range_max=1.0 --fgsm_balance_factor=0.6 --modified_fgsm_dim_selection="gradient_based" --num_abstractions=1000

#### GTSRB
bazel-bin/tensorflow/ARFramework/ARFramework_FGSM_test --graph="gtsrb_gradient.pb" --root_dir=/home/jsmith/tensorflow/tensorflow/ARFramework/gtsrb --initial_activation=gtsrb_200.pb --input_layer="input_layer_x" --output_layer="probabilities_out" --gradient_layer="gradient_out" --granularity=0.00390625 --verification_radius=0.4 --class_averages=gtsrb_averages.pb --label_proto=gtsrb_200_label.pb --label_layer="label_layer_y" --enforce_domain=true --domain_range_min=0.0 --domain_range_max=1.0 --fgsm_balance_factor=0.6 --modified_fgsm_dim_selection="gradient_based" --num_abstractions=1000

