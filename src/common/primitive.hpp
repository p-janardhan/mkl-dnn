#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include "mkl_dnn.h"

#include "c_types_map.hpp"
#include "nstl.hpp"

namespace mkl_dnn { namespace impl { struct memory; }}

// TODO: consider using smart pointers for storing primitives. External handles
// then would have to be cast to smart pointers. This would ensure that
// accedentally deleting a primitive that is a dependency for another one does
// not cause a segfault.

struct mkl_dnn_primitive: public mkl_dnn::impl::c_compatible {
private:
    // TODO: copy, equality and assignment -- all must be banned...
protected:
    mkl_dnn::impl::engine *_engine;
    mkl_dnn::impl::primitive_kind_t _kind;
    mkl_dnn::impl::nstl::vector<mkl_dnn::impl::primitive*> _input;
    mkl_dnn::impl::nstl::vector<mkl_dnn::impl::primitive*> _output;

    virtual mkl_dnn::impl::status_t execute_impl() = 0;

    mkl_dnn_primitive(mkl_dnn::impl::engine *engine,
			mkl_dnn::impl::primitive_kind_t kind)
        : _engine(engine)
        , _kind(kind) {}
public:
    virtual ~mkl_dnn_primitive() {}

    mkl_dnn::impl::primitive_kind_t kind() const { return _kind; }
    mkl_dnn::impl::engine *engine() const { return _engine; }

    virtual bool own_memory() const { return false; }

    enum exec_state { done, busy, not_ready, error };
    virtual exec_state get_exec_state() const = 0;
    bool inputs_ready() {
        for (auto i = 0UL; i < _input.size(); i++)
            if (_input[i]->get_exec_state() != done)
                return false;
        return true;
    }
    mkl_dnn::impl::status_t execute() {
        if (!inputs_ready())
            return mkl_dnn_not_ready;
        return execute_impl();
    }

    size_t input_count() const { return _input.size(); }
    mkl_dnn::impl::nstl::vector<mkl_dnn::impl::primitive*> &input()
	{ return _input; }

    size_t output_count() const { return _output.size(); }
    mkl_dnn::impl::nstl::vector<mkl_dnn::impl::primitive*> &output()
	{ return _output; }

    // XXX: memory -> primitive?
    virtual const mkl_dnn::impl::memory *output_memory_const(
            size_t at = 0) const {
        return _output[at]->memory_const();
    }
    virtual mkl_dnn::impl::memory *output_memory(size_t at = 0) const {
        return _output[at]->memory();
    }

    virtual mkl_dnn::impl::memory *memory() { return 0; }
    virtual mkl_dnn::impl::memory *memory_const() { return 0; }
};

namespace mkl_dnn { namespace impl {

typedef const void* const_op_desc_t;
typedef mkl_dnn_status_t (*primitive_desc_init_f)(
		primitive_desc_t *primitive_desc,
        const_op_desc_t op_desc,
		const engine &aengine);
typedef mkl_dnn_status_t (*primitive_create_f)(
		primitive **aprimitive,
        const_primitive_desc_t primitive_desc,
        const primitive_at_t inputs[],
		const primitive *outputs[]);

struct primitive_impl /* : public c_compatible */ {
    const primitive_desc_init_f primitive_desc_init;
    const primitive_create_f primitive_create;
};

mkl_dnn_status_t inline check_inputs_array(size_t n,
		const mkl_dnn_primitive_at_t inputs[]) {
    for (size_t i = 0; i < n; i++)
        if (inputs[i].primitive->output_count() <= inputs[i].output_index)
            return mkl_dnn_invalid_arguments;
    return mkl_dnn_success;
}

struct memory: public mkl_dnn_primitive { };

}}

#endif

// vim: et ts=4 sw=4 cindent cino^=l0,\:0