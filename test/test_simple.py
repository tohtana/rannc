import pytest

from . import common, models
# from . import native_models

default_vals = {
    "rtol": 1e-1,
    "atol": 0,
    "get_dataset": None,
    "loss_out": False,
    "preprocess": None
}

test_models = [
    # {"model": models.BasicModel, "rtol": 1e-1},
    {"model": models.SmallParamModel},
    {"model": models.ForkJoinModel},
    {"model": models.SharedParamModel},
    {"model": models.OneOpModel},
    {"model": models.TensorMulModel},
    {"model": models.EmbeddingModel, "get_dataset": models.EmbeddingModel.get_dataset},
    {"model": models.FunctionModel, "get_dataset": models.FunctionModel.get_dataset},
    {"model": models.LossOutModel, "loss_out": True},
    # {"model": native_models.NativeCallModel01}, # compiles module
    # {"model": models.LayerNormModel, "preprocess": models.norm_to_float} # DP only
]


@pytest.mark.parametrize("test_model", test_models)
@pytest.mark.parametrize("use_amp", [False, True])
@pytest.mark.parametrize("allreduce_amp_master_params", [False, True])
@pytest.mark.parametrize("enable_zero", [False, True])
@pytest.mark.parametrize("dist_params", [False, True])
def test_match(init_dist, init_seed, batch_size, iteration, test_model, use_amp, allreduce_amp_master_params,
               enable_zero, dist_params):

    if enable_zero and (not allreduce_amp_master_params):
        print("allreduce_amp_master_params must be True if enable_zero == True")
        return

    print("use_amp={} allreduce_amp_master_params={} enable_zero={} dist_params={}".format(
          use_amp, allreduce_amp_master_params, enable_zero, dist_params))

    for k, v in default_vals.items():
        if k not in test_model:
            test_model[k] = v

    common.run(test_model["model"], batch_size, iteration,
               loss_out=test_model["loss_out"],
               preprocess=test_model["preprocess"],
               use_amp=use_amp,
               allreduce_amp_master_params=allreduce_amp_master_params,
               enable_zero=enable_zero,
               dist_params=dist_params,
               rtol=test_model["rtol"],
               atol=test_model["atol"],
               get_dataset=test_model["get_dataset"]
               )

