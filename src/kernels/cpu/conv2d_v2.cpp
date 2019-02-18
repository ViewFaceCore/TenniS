#include <kernels/cpu/conv2d_v2.h>
#include <global/operator_factory.h>
#include <backend/name.h>

#include "utils/ctxmgr_lite.h"
#include "core/device_context.h"

#include "module/bubble.h"
#include "core/tensor_builder.h"

namespace ts {
    namespace cpu {

        Conv2DV2::Conv2DV2() {
            field(name::format, REQUIRED);
            field(name::padding_value, OPTIONAL, tensor::from(0.0f));
            field(name::stride, REQUIRED);
            field(name::dilation, OPTIONAL);
            field(name::typo::dialations, OPTIONAL);
        }

        void Conv2DV2::init() {
            supper::init();

            auto &context = ctx::ref<DeviceContext>();

            m_op_conv2d = OperatorCreator::Create(context.computing_device.type(), name::layer::conv2d(), false);

            TS_CHECK_NQ(m_op_conv2d, nullptr) << "Can not find operator: " << name::layer::conv2d();

            m_op_conv2d->set(Bubble::RetentionParam::op, tensor::from(name::layer::conv2d_v2()));
            m_op_conv2d->set(Bubble::RetentionParam::name, tensor::from("_core" + name()));
            m_op_conv2d->set(Bubble::RetentionParam::output_count, get(Bubble::RetentionParam::output_count));

            m_op_conv2d->set(name::format, get(name::format));
            m_op_conv2d->set(name::padding_value, get(name::padding_value));
            m_op_conv2d->set(name::stride, get(name::stride));

            if (has(name::dilation)) m_op_conv2d->set(name::dilation, get(name::dilation));
            if (has(name::typo::dialations)) m_op_conv2d->set(name::typo::dialations, get(name::typo::dialations));
        }

        static bool is_int_equal(const Tensor &lhs, const Tensor &rhs) {
            if (!lhs.has_shape(rhs.sizes())) return false;
            auto count = lhs.count();
            for (int i = 0; i < count; ++i) {
                if(lhs.data<int>(i) != rhs.data<int>(i)) return false;
            }
            return true;
        }

        int Conv2DV2::infer(Stack &stack, std::vector<Tensor::Prototype> &output) {
            TS_AUTO_CHECK(stack.size() == 3);

            auto padding = tensor::cast(INT32, stack[1]);

            if (!is_int_equal(padding, m_int_padding4x2)) {
                m_int_padding4x2 = padding.clone();
                m_op_conv2d->set(name::padding, m_int_padding4x2);
                m_op_conv2d->init();
            }

            return m_op_conv2d->infer(stack, output);
        }

        int Conv2DV2::run(Stack &stack) {
            TS_AUTO_CHECK(stack.size() == 3);

            auto padding = tensor::cast(INT32, stack[1]);

            if (!is_int_equal(padding, m_int_padding4x2)) {
                m_int_padding4x2 = padding.clone();
                m_op_conv2d->set(name::padding, m_int_padding4x2);
                m_op_conv2d->init();
            }

            return m_op_conv2d->run(stack);
        }
    }
}

using namespace ts;
using namespace cpu;
TS_REGISTER_OPERATOR(Conv2DV2, CPU, name::layer::conv2d_v2())
