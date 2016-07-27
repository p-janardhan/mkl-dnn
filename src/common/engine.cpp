#include "mkl_dnn.h"
#include "engine.hpp"
#include "nstl.hpp"

#include "c_types_map.hpp"
#include "../cpu/cpu_engine.hpp"

namespace mkl_dnn { namespace impl {

// TODO: we need some caching+refcounting mechanism so that an engine could not
// be created twice and is only destroyed when the refcount is 0

// With STL we would've used vector. Alas, we cannot use STL..
engine_factory *engine_factories[] = {
    &cpu::engine_factory,
    &cpu::engine_factory_lazy,
    NULL,
};

static inline engine_factory *get_engine_factory(engine_kind_t kind)
{
    for (engine_factory **ef = engine_factories; *ef; ef++)
        if ((*ef)->kind() == kind)
            return *ef;
    return NULL;
}

}}

namespace {
    mkl_dnn::impl::primitive_desc_init_f empty_list[] = { nullptr };
}

using namespace mkl_dnn::impl;

primitive_desc_init_f *engine::get_memory_inits() const {
    return empty_list;
}
primitive_desc_init_f *engine::get_reorder_inits() const {
    return empty_list;
}
primitive_desc_init_f *engine::get_convolution_inits() const
{
    return empty_list;
}

size_t mkl_dnn_engine_get_count(engine_kind_t kind) {
    engine_factory *ef = get_engine_factory(kind);
    return ef != NULL ? ef->count() : 0;
}

status_t mkl_dnn_engine_create(mkl_dnn_engine_t *engine,
		mkl_dnn_engine_kind_t kind, size_t index) {
    if (engine == NULL)
        return mkl_dnn_invalid_arguments;

    engine_factory *ef = get_engine_factory(kind);
    if (ef == NULL || index >= ef->count())
        return mkl_dnn_invalid_arguments;

    return ef->engine_create(engine, index);
}

status_t mkl_dnn_engine_destroy(mkl_dnn_engine_t engine) {
    delete engine;
	return mkl_dnn_success;
}

// vim: et ts=4 sw=4 cindent cino^=l0,\:0