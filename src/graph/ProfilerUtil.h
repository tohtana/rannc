//
// Created by Masahiro Tanaka on 2020/04/11.
//

#ifndef PYRANNC_PROFILERUTIL_H
#define PYRANNC_PROFILERUTIL_H

#include <comp/GraphProfiler.h>
#include <distop/PartitionTensor.h>
#include "ir.h"

namespace rannc {

size_t calcInputSize(const std::shared_ptr<IRGraph>& g);
size_t calcOutputSize(const std::shared_ptr<IRGraph>& g);
long calcCommTime(long cut_size);
long calcInputCommTime(const std::shared_ptr<IRGraph>& g, int repl);
long calcOutputCommTime(const std::shared_ptr<IRGraph>& g, int repl);
long calcAllReduceTime(long cut_size);

size_t calcGraphMem(
    const std::shared_ptr<IRGraph>& g, const GraphProfile& prof,
    bool use_amp_master_params, bool enable_zero, int zero_dist_num);
size_t calcGraphMem(
    const std::shared_ptr<IRGraph>& g, const GraphProfile& prof,
    size_t batch_size, int replica_num, int pipeline_num,
    bool use_amp_master_params, bool enable_zero);
bool fitToMem(
    const std::shared_ptr<IRGraph>& g, const GraphProfile& prof, long capacity,
    bool use_amp_master_params, bool enable_zero, int zero_dist_num);

struct MLProfileKey {
  std::string id;
  size_t batch_size;
  bool checkpointing;

  bool operator==(const MLProfileKey& rhs) const {
    return id == rhs.id && batch_size == rhs.batch_size &&
        checkpointing == rhs.checkpointing;
  }

  bool operator!=(const MLProfileKey& rhs) const {
    return !(rhs == *this);
  }

  MSGPACK_DEFINE(id, batch_size, checkpointing);
};

struct MLProfileKeyHash {
  std::size_t operator()(const MLProfileKey& key) const {
    std::stringstream ss;
    ss << key.id << "_" << key.batch_size << "_cp=" << key.checkpointing;
    return std::hash<std::string>()(ss.str());
  };
};

using MLProfileCache =
    std::unordered_map<MLProfileKey, GraphProfile, MLProfileKeyHash>;

class ProfilerUtil {
 public:
  ProfilerUtil(std::shared_ptr<GraphProfiler> profiler)
      : profiler_(std::move(profiler)), dist_profile_(false){};
  ProfilerUtil(std::shared_ptr<GraphProfiler> profiler, bool dist_profile)
      : profiler_(std::move(profiler)), dist_profile_(dist_profile){};

  GraphProfile profile(
      const std::shared_ptr<IRGraph>& g, size_t batch_size, size_t replica_num,
      size_t pipeline_num, bool checkpointing = false);
  GraphProfile profileDist(
      const TensorPartioningGraphInfo& part_info, size_t batch_size,
      size_t replica_num, size_t pipeline_num, bool checkpointing);

  const MLProfileCache& getProfileCache() const {
    return profile_cache_;
  }

  void setProfileCache(const MLProfileCache& profileCache) {
    profile_cache_ = profileCache;
  }

  static const long ERROR_VAL = LONG_MAX / 1024;

 private:
  GraphProfile doProfile(
      const std::shared_ptr<IRGraph>& g, size_t batch_size, size_t replica_num,
      size_t pipeline_num, bool checkpointing,
      const TensorPartioningGraphInfo& part_info,
      const std::function<ProfilingResult(const ProfilingInput& input)>& f);

  MLProfileCache profile_cache_;
  std::unordered_map<bool, std::unordered_map<std::string, size_t>>
      max_batch_size_cache_;
  std::shared_ptr<GraphProfiler> profiler_;
  bool dist_profile_;

  static const int DEFALUT_ITERATION_NUM;
};

GraphProfile accProfileValues(
    ProfilerUtil& prof_util, size_t batch_size,
    const std::vector<std::shared_ptr<IRGraph>>& graphs, size_t from, size_t to,
    size_t dev_num, size_t pipeline_num, bool checkpointing);

std::string displayGraphProfiles(
    const std::vector<std::shared_ptr<IRGraph>>& graphs, size_t batch_size,
    int pipeline_num, bool use_amp_master_params, bool enable_zero,
    const std::unordered_map<std::string, int>& repl_nums,
    const std::unordered_map<std::string, GraphProfile>& profiles);
} // namespace rannc
#endif // PYRANNC_PROFILERUTIL_H
