#ifndef TS_KERNELS_BATCH_SCALE_H
#define TS_KERNELS_BATCH_SCALE_H

#include <core/tensor.h>
#include <runtime/stack.h>
#include <runtime/operator.h>


namespace ts {


class Batch_Scale : public ts::Operator {
public:

    using supper = ts::Operator;
    Batch_Scale();

    virtual void init(); 

    virtual int run(ts::Stack &stack);
    virtual int infer(ts::Stack &stack, std::vector<ts::Tensor::Prototype> &output); 


private:
    template<typename T>
    void compute_batch_scale(Tensor *input_tensor, Tensor *scale_tensor,
                             Tensor *bias_tensor, Tensor *output_tensor);
    void infer_private(ts::Stack &stack, ts::Tensor::Prototype &output);

private:
    int m_dim;

};





}

#endif
