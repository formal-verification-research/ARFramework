#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>

#include "grid_tools.hpp"

grid::region grid::snapToDomainRange(
        grid::region const& r,
        grid::region const& range)
{
    grid::region retVal(r);
    for(auto i = 0u; i < r.size(); ++i)
    {
        if(retVal[i].first < range[i].first) retVal[i].first = range[i].first;
        if(retVal[i].second < range[i].first) retVal[i].second = range[i].first;
        if(retVal[i].first > range[i].second) retVal[i].first = range[i].second;
        if(retVal[i].second > range[i].second) retVal[i].second = range[i].second;
    }
    return retVal;
}

grid::point grid::snapToDomainRange(
        grid::point const& p,
        grid::region const& r)
{
    grid::point retVal(p);
    for(auto i = 0u; i < p.size(); ++i)
    {
        if(retVal[i] < r[i].first) retVal[i] = r[i].first;
        else if(retVal[i] > r[i].second) retVal[i] = r[i].second;
    }
    return retVal;
}

bool grid::isInDomainRange(
        grid::point const& p, 
        grid::region const& range)
{
    for(auto i = 0u; i < p.size(); ++i)
    {
        if(p[i] < range[i].first ||
                p[i] > range[i].second)
            return false;
    }
    return true;
}

bool grid::isInDomainRange(
        grid::region const& r, 
        grid::region const& range)
{
    for(auto i = 0u; i < r.size(); ++i)
    {
        if(r[i].first < range[i].first ||
                r[i].second < range[i].first ||
                r[i].first > range[i].second ||
                r[i].second > range[i].second)
            return false;
    }
    return true;
}

grid::point grid::enforceSnapDiscreteGrid(
        grid::point const& p, 
        grid::point const& referencePoint, 
        grid::point const& granularity)
{
    grid::point retVal(p.size());
    for(auto i = 0u; i < retVal.size(); ++i)
    {
        auto multiplier = 
            round((p[i] - referencePoint[i]) / granularity[i]);
        retVal[i] = referencePoint[i] + multiplier*granularity[i];
    }
    return retVal;
}

bool grid::isValidRegion(grid::region const & r)
{
    for(auto&& range : r)
    {
        if(range.first > range.second) return false;
    }
    return true;
}

bool grid::pointIsInRegion(grid::region const& r, grid::point const& p)
{
    for(auto i = 0u; i < r.size(); ++i)
        if(p[i] < r[i].first || p[i] >= r[i].second)
            return false;
    return true;
}

long double grid::regionVolume(grid::region const& r)
{
    return std::accumulate(r.begin(), r.end(), static_cast<long double>(1.0), 
        [](grid::numeric_type_t acc, grid::region_element const& elem)
            {
                return acc*(elem.second - elem.first);
            });
}

grid::VolumeThresholdFilterStrategy::VolumeThresholdFilterStrategy(
        grid::numeric_type_t t)
    : threshold(t)
{
}

bool 
grid::VolumeThresholdFilterStrategy::operator()(grid::region const& r)
{
    auto volume = grid::regionVolume(r);
    return volume < threshold;
}

grid::RandomPointRegionAbstraction::RandomPointRegionAbstraction(
        unsigned n)
    : numPoints(n), generator(1029)
{
}

grid::abstraction_strategy_return_t 
grid::RandomPointRegionAbstraction::operator()(region const& r)
{
    grid::abstraction_strategy_return_t retVal;
    //std::default_random_engine generator;
    for(auto i = 0u; i < numPoints; ++i)
    {
        grid::point tmp(r.size());
        for(auto j = 0u; j < r.size(); ++j)
        {
            std::uniform_real_distribution<grid::numeric_type_t>
                dist(r[j].first, r[j].second);
            tmp[j] = dist(generator);
        }
        retVal.push_back(tmp);
    }
    return retVal;
}

grid::abstraction_strategy_return_t 
grid::centralPointRegionAbstraction(grid::region const& r)
{
    grid::point p(r.size());
    for(auto i = 0u; i < r.size(); ++i)
    {
        p[i] = 
            (r[i].first + r[i].second) 
            / static_cast<grid::numeric_type_t>(2.0);
    }
    return {p};
}

grid::AllValidDiscretizedPointsAbstraction::AllValidDiscretizedPointsAbstraction(
        grid::point vp, 
        grid::point gran)
    : knownValidPoint(vp), granularity(std::abs(gran))
{
}

grid::abstraction_strategy_return_t 
grid::AllValidDiscretizedPointsAbstraction::operator()(
        grid::region const& r)
{
    auto validPoint = findValidPointInRegion(r);
    if(!validPoint.first) return {};
    grid::abstraction_strategy_return_t retVal;
    enumerateAllPoints(retVal, validPoint.second, 0u, r);
    return retVal;
}

// protected with overflow check, result may be too large to be exact
unsigned long long 
grid::AllValidDiscretizedPointsAbstraction::getNumberValidPoints(
        grid::region const& r,// region in question 
        grid::point const& p, // valid point
        grid::point const& g) // granularity
{
    auto validPointInRegion = findValidPointInRegion(r, p, g);
    if(!validPointInRegion.first) 
    {
        return 0ull;
    }
    auto retVal = 1ull;
    for(auto i = 0u; i < r.size(); ++i)
    {
        if(r[i].first == r[i].second) continue;
        auto numPointsInDim =  
            static_cast<unsigned long long>(
                    ceil((r[i].second - validPointInRegion.second[i])
                    / g[i]));
        if(numPointsInDim == 0ull) return 0;
        if(retVal < 
                std::numeric_limits<unsigned long long>::max() 
                / numPointsInDim)
        {
            retVal *= numPointsInDim;
        }
    }
    return retVal;
}

unsigned long long 
grid::AllValidDiscretizedPointsAbstraction::getNumberValidPoints(
        grid::region const& r)
{
    return getNumberValidPoints(r, knownValidPoint, granularity);
}

std::pair<bool, grid::point> 
grid::AllValidDiscretizedPointsAbstraction::findValidPointInRegion(
        grid::region const& r,
        grid::point const& p,
        grid::point const& g)
{
    grid::point retVal(r.size());
    for(auto i = 0u; i < r.size(); ++i)
    {
        auto multiplier = 
            ceil((r[i].first - p[i]) / g[i]);
        auto value = p[i] + multiplier*g[i]; 
        if(value >= r[i].second && value != r[i].first)
            return {false, {}};
        retVal[i] = value;
    }
    return {true, retVal};
}

std::pair<bool, grid::point> 
grid::AllValidDiscretizedPointsAbstraction::findValidPointInRegion(
        grid::region const& r)
{
    return findValidPointInRegion(r, knownValidPoint, granularity);
}

bool 
grid::AllValidDiscretizedPointsAbstraction::enumerateAllPoints(
        grid::abstraction_strategy_return_t& s, 
        grid::point const& p, 
        std::size_t curIndex,
        grid::region const& r)
{
    if(curIndex >= r.size()) return true;
    auto newp = p;
    while(newp[curIndex] < r[curIndex].second)
    {
        if(enumerateAllPoints(s, newp, curIndex+1, r))
            s.push_back(newp);
        if(granularity[curIndex] <= 0.0) break;
        newp[curIndex] += granularity[curIndex];
    }
    return false;
}

grid::DiscreteSearchVerificationEngine::DiscreteSearchVerificationEngine(
        std::function<bool(grid::region const&)> const& shouldAttempt,
        grid::region_abstraction_strategy_t const& dpg,
        std::function<bool(point const&)> const& point_safe)
    : shouldAttemptCheck(shouldAttempt), 
    discretePointGenerator(dpg), 
    point_safe_func(point_safe)
{
}

grid::verification_engine_return_t
grid::DiscreteSearchVerificationEngine::operator()(
        grid::region const& r)
{
    if(!shouldAttemptCheck(r)) 
    {
        return {grid::VERIFICATION_RETURN::UNKNOWN, {}};
    }
    auto points = discretePointGenerator(r);
    for(auto&& p : points)
    {
        if(!point_safe_func(p))
            return {grid::VERIFICATION_RETURN::UNSAFE, p};
    }
    return {grid::VERIFICATION_RETURN::SAFE, {}};
}

grid::dim_selection_strategy_return_t
grid::randomDimSelection(region const& r, std::size_t numDims)
{
    static std::minstd_rand0 rd(133);
    static std::mt19937 g(rd());
    grid::dim_selection_strategy_return_t indices(r.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), g);
    return {indices.begin(), indices.begin() + numDims};
}

grid::dim_selection_strategy_return_t
grid::largestDimFirst(region const& r, std::size_t numDims)
{
    grid::dim_selection_strategy_return_t indices(r.size());
    std::iota(indices.begin(), indices.end(), 0u);
    std::sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b)
            {
                return r[a].second - r[a].first > r[b].second - r[b].first;
            });
    return {indices.begin(), indices.begin() + numDims};
}

grid::dim_selection_strategy_return_t
grid::maxAverageDimSelection(grid::region const& r, std::size_t numDims)
{
    if(numDims > r.size()) 
        return {};
    auto centralPoint = grid::centralPointRegionAbstraction(r);
    if(centralPoint.empty()) return {};
    auto p = *centralPoint.begin();
    auto avg = std::accumulate(p.begin(), p.end(), static_cast<grid::numeric_type_t>(0))
        / static_cast<grid::numeric_type_t>(p.size());
    grid::point dif(p.size());
    for(auto i = 0u; i < p.size(); ++i)
    {
        dif[i] = std::abs(avg - p[i]);
    }
    auto indices = getSortedIndices(dif, false /* descending */);
    return {indices.begin(), indices.begin() + numDims};
}

grid::GradientBasedDimensionSelection
    ::GradientBasedDimensionSelection(
            std::function<point(point const&)> const& gradient)
    : grad(gradient)
{
}

grid::dim_selection_strategy_return_t
grid::GradientBasedDimensionSelection::operator()(
        grid::region const& r, std::size_t numDims)
{
    if(numDims > r.size()) 
        return {};
    auto centralPoint = grid::centralPointRegionAbstraction(r);
    if(centralPoint.empty()) return {};
    auto p = *centralPoint.begin();
    auto gradient = grad(p);
    for(auto&& elem : gradient)
        elem = std::abs(elem);
    auto indices = getSortedIndices(gradient, false);
    return {indices.begin(), indices.begin() + numDims};
}

long double grid::l2norm(grid::point const& p)
{
    grid::numeric_type_t squared_sum = 0;
    for(auto&& elem : p)
        squared_sum += elem*elem;
    return sqrt(squared_sum);
}

grid::IntellifeatureDimSelection::IntellifeatureDimSelection(
        std::vector<point> const& avgs,
        norm_function_type_t norm,
        std::size_t orig_cl)
    : averages(avgs), norm_func(norm), orig_class(orig_cl)
{
}

std::vector<std::size_t> 
grid::IntellifeatureDimSelection::operator()(grid::region const& r, std::size_t numDims)
{
    if(numDims > r.size())
        return {};
    // get central point of region
    auto centralPointSet = grid::centralPointRegionAbstraction(r);
    if(centralPointSet.empty()) return {};
    auto p = *centralPointSet.begin();
    // calculate the norms between the averages and the central point
    std::vector<double> norms(averages.size());
    for(auto i = 0u; i < averages.size(); ++i)
    {
        // skip the original class
        if(i == orig_class) continue;
        norms[i] = norm_func(averages[i] - p);
    }
    auto indices = getSortedIndices(norms);
    std::remove(indices.begin(), indices.end(), orig_class);
    indices.push_back(orig_class);
    auto minDiff = std::abs(p - averages[indices[0]]);
    indices = getSortedIndices(minDiff, false /* descending */);
    return {indices.begin(), indices.begin() + numDims};
}

grid::point grid::sign(grid::point const& p)
{
    grid::point ret(p.size(), 0.0);
    for(auto i = 0u; i < p.size(); ++i)
    {
        if(p[i] < 0) ret[i] = -1;
        else if(p[i] > 0) ret[i] = 1;
    }
    return ret;
}

grid::ModifiedFGSMWithFallbackRegionAbstraction::ModifiedFGSMWithFallbackRegionAbstraction(
        std::size_t mp, 
        std::function<grid::point(grid::point const&)> const& grad,
        grid::dimension_selection_strategy_t const& dim_sel,
        grid::region_abstraction_strategy_t const& fallback_strategy,
        grid::point const& granularity,
        double pFGSM)
    : maxPoints(mp), gradient(grad), 
      dim_select_strategy(dim_sel),
      fallback_strategy(fallback_strategy),
      granularity(granularity),
      percentFGSM(std::abs(pFGSM) > 1 ? 1 : std::abs(pFGSM)),
      rand_gen(42)
{
}

grid::abstraction_strategy_return_t
grid::ModifiedFGSMWithFallbackRegionAbstraction::operator()(grid::region const& r)
{
    auto centralPointSet = grid::centralPointRegionAbstraction(r);
    if(centralPointSet.empty()) return {};

    auto p = *centralPointSet.begin();
    auto grad_sign = grid::sign(gradient(p));

    auto min_dimension = 
        std::min_element(r.begin(), r.end(),
                [](grid::region_element const& a,
                    grid::region_element const& b)
                { return a.second - a.first < b.second - b.first; });

    auto min_dimension_index = std::distance(r.begin(), min_dimension);
    auto max_radius = 
        (min_dimension->second - min_dimension->first) / (long double)1.25;
    if(r.end() == min_dimension || 
            max_radius <= granularity[min_dimension_index])
    {
        return fallback_strategy(r);
    }

    auto dims = dim_select_strategy(r, r.size());
    auto numDimsFGSM = 
        static_cast<unsigned>(
                percentFGSM * static_cast<double>(r.size()));
    grid::point M(p.size(), 0.0);
    grid::point Mnot(p.size(), 1.0);
    for(auto i = 0u; i < numDimsFGSM; ++i)
    {
        M[dims[i]] = 1.0;
        Mnot[dims[i]] = 0.0;
    }

    auto e1_lowerbound = 1;
    auto e1_upperbound = 
        static_cast<int>(max_radius / granularity[min_dimension_index]);

    //std::default_random_engine rand_gen;
    auto dist_R = std::uniform_int_distribution<int>(0,1);
    auto e1_dist = std::uniform_int_distribution<int>(
            e1_lowerbound, e1_upperbound);

    grid::abstraction_strategy_return_t retVal;
    retVal.reserve(maxPoints);
    for(int i = 0; i < maxPoints; ++i)
    {
        auto e1 = 
            granularity[min_dimension_index] * 
            static_cast<long double>(e1_dist(rand_gen));
        grid::point R(r.size());
        for(auto&& elem : R)
        {
            elem = dist_R(rand_gen);
            if(elem != 1) elem = -1;
        }
        auto fgsm_part = elementWiseMult(grad_sign, M);
        auto mod_part = elementWiseMult(R, Mnot);
        auto scaled_part = constVecMult(e1, fgsm_part + mod_part);
        auto generated_point = p + scaled_part;
        retVal.push_back(generated_point);
    }
    return retVal;
}

grid::RPlusFGSMAbstraction::RPlusFGSMAbstraction(
                std::size_t n,
                std::function<point(point const&)> const& grad,
                point const& gran,
                double eps) 
    : maxPoints(n), gradient(grad), granularity(gran), rand_gen(42),
    epsilon(eps)
{
}

grid::abstraction_strategy_return_t 
grid::RPlusFGSMAbstraction::operator()(grid::region const& r)
{
    auto centralPointSet = grid::centralPointRegionAbstraction(r);
    if(centralPointSet.empty()) return {};

    auto p = *centralPointSet.begin();
    for(int i = 0; i < p.size(); ++i)
    {
        long double radius = (r[i].second - r[i].first) / 2.0;
        std::uniform_real_distribution<> dist(-radius, radius);
        double mod = dist(rand_gen);
        p[i] += mod;
    }
    auto grad_sign = grid::sign(gradient(p));
    grid::abstraction_strategy_return_t retVal;
    retVal.reserve(maxPoints);
    auto min_dimension = 
        std::min_element(r.begin(), r.end(),
                [](grid::region_element const& a,
                    grid::region_element const& b)
                { return a.second - a.first < b.second - b.first; });
    auto min_dimension_index = std::distance(r.begin(), min_dimension);
    auto max_radius = 
        (min_dimension->second - min_dimension->first) / (long double)1.25;

    auto e1_lowerbound = 1;
    auto e1_upperbound = 
        static_cast<int>(max_radius / granularity[min_dimension_index]);

    for(int i = 0; i < maxPoints; ++i)
    {
        auto e1_dist = std::uniform_int_distribution<int>(
                e1_lowerbound, e1_upperbound);

        auto cur_eps = granularity[min_dimension_index] * static_cast<long double>(e1_dist(rand_gen));
        auto gen_point = p;
        for(int j = 0; j < p.size(); ++j)
        {
            gen_point[j] += (cur_eps * grad_sign[j]);
        }
        retVal.push_back(gen_point);
    }
    return retVal;
}

grid::HierarchicalDimensionRefinementStrategy::HierarchicalDimensionRefinementStrategy(
        grid::dimension_selection_strategy_t const& dim_select,
        unsigned divisor,
        unsigned ndims)
    : dim_select_strategy(dim_select), dim_divisor(divisor), numDims(ndims)
{
}

grid::refinement_strategy_return_t
grid::HierarchicalDimensionRefinementStrategy::operator()(grid::region const& r)
{
    auto dims = dim_select_strategy(r, numDims);
    grid::refinement_strategy_return_t retVal;
    auto firstRegion = r;
    for(auto i = 0u; i < dims.size(); ++i)
    {
        auto curIndex = dims[i];
        auto sizeIncrement = (r[curIndex].second - r[curIndex].first)
            / (grid::numeric_type_t)dim_divisor;
        firstRegion[curIndex].second = firstRegion[curIndex].first + sizeIncrement;
    }
    enumerateAllRegions(retVal, firstRegion, 0, dims, r);
    return retVal;
}

bool
grid::HierarchicalDimensionRefinementStrategy::enumerateAllRegions(
        grid::refinement_strategy_return_t& s,
        grid::region const& r,
        unsigned index,
        grid::dim_selection_strategy_return_t const& selected_dims,
        grid::region const& orig_region)
{
    if(index >= selected_dims.size()) return true;
    auto newr = r;
    auto curIndex = selected_dims[index];
    auto diff = newr[curIndex].second - newr[curIndex].first;
    while(newr[curIndex].second <= orig_region[curIndex].second)
    {
        if(enumerateAllRegions(s, newr, index+1, selected_dims, orig_region))
            s.insert(newr);
        if(diff <= 0.0) break;
        newr[curIndex].first = newr[curIndex].second;
        newr[curIndex].second += diff;
    }
    return false;
}

bool operator<(grid::point const& p, grid::region const& r)
{
    for(auto i = 0u; i < p.size(); ++i)
    {
        if(p[i] < r[i].first) return true;
        if(p[i] >= r[i].second) return false;
    }
    return false;
}

bool operator<(grid::region const& r, grid::point const& p)
{
    for(auto i = 0u; i < p.size(); ++i)
    {
        if(r[i].second <= p[i]) return true;
        if(r[i].first > p[i]) return false;
    }
    return false;
}

bool operator<(grid::region const& r1, grid::region const& r2)
{
    for(auto i = 0u; i < r1.size(); ++i)
    {
        if(r1[i].second <= r2[i].first) return true;
        if(r1[i].first >= r2[i].second) return false;
    }
    return false;
}

bool operator<(grid::point const& p1, grid::point const& p2)
{
    for(auto i = 0u; i < p1.size(); ++i)
    {
        if(p1[i] < p2[i]) return true;
        if(p1[i] > p2[i]) return false;
    }
    return false;
}

grid::point operator-(grid::point const& a, grid::point const& b)
{
    grid::point retVal(a.size());
    for(auto i = 0u; i < a.size(); ++i)
    {
        retVal[i] = a[i] - b[i];
    }
    return retVal;
}

grid::point operator+(grid::point const& a, grid::point const& b)
{
    grid::point retVal(a.size());
    for(auto i = 0u; i < a.size(); ++i)
    {
        retVal[i] = a[i] + b[i];
    }
    return retVal;
}

grid::point elementWiseMult(grid::point const& a, grid::point const& b)
{
    grid::point retVal(a.size());
    for(auto i = 0u; i < a.size(); ++i)
        retVal[i] = a[i] * b[i];
    return retVal;
}

grid::point std::abs(grid::point const& p)
{
    grid::point ret(p.size());
    for(auto i = 0u; i < p.size(); ++i)
        ret[i] = std::abs(p[i]);
    return ret;
}

std::ostream& operator<<(std::ostream& os, grid::region const& r)
{
    os << "| ";
    for(auto&& range : r) 
        os << "(" << range.first << "," << range.second << ")" << " | ";
    return os;
}

std::ostream& operator<<(std::ostream& os, grid::point const& p)
{
    os << "| ";
    for(auto&& r : p)
        os << "(" << r << ")" << " | ";
    return os;
}

std::ostream& operator<<(std::ostream& os, 
        grid::VERIFICATION_RETURN const& v)
{
    switch(v)
    {
    case grid::VERIFICATION_RETURN::SAFE:
        os << "SAFE"; break;
    case grid::VERIFICATION_RETURN::UNSAFE:
        os << "UNSAFE"; break;
    case grid::VERIFICATION_RETURN::UNKNOWN:
        os << "UNKNOWN"; break;
    }
    return os;
}


