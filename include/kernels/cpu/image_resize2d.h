#ifndef TS_KERNELS_IMAGE_RESIZE2D_H
#define TS_KERNELS_IMAGE_RESIZE2D_H

#include <global/operator_factory.h>
#include <core/tensor.h>
#include <runtime/stack.h>
#include <cstring>
#include <math.h>



namespace ts {


//support UINT8,UINT16,FLOAT32,FLOAT64, channel 1,2,3
class Image_Resize2d : public ts::Operator {
public:


    template<typename T>
    static void ResizeImageLinear( const T *src_im, int src_width, int src_height, int channels,
                                     T *dst_im, int dst_width, int dst_height, bool ishwc);

    template<typename T>
    static void ResizeImageCubic(const T *src_im, int src_width, int src_height, int channels,
                             T *dst_im, int dst_width, int dst_height, bool ishwc);

public:
    enum RESIZETYPE{
        linear,
        cublic
    };

    using supper = ts::Operator;
    Image_Resize2d() {
        field("type", OPTIONAL);
    }

    virtual void init() {
        supper::init();
    }

    virtual int run(ts::Stack &stack) {
        int input_num = stack.size();

        if(input_num != 2 || stack.index(0)->dims() < 2 || stack.index(0)->dims() != stack.index(1)->count()) {
            throw ts::Exception("input parameters is invalid");
        }

        int type_len = ts::type_bytes(stack.index(0)->dtype());

        if(stack.index(1)->dtype() != ts::INT32) {
            throw ts::Exception("input parameters 1 only support INT32 type");
        }
        int dims = stack.index(0)->dims();
        int i=0;
        for(i=0; i< dims; i++) {
            if((int)(stack.index(1)->data<int>()[i]) > 0)
                break;
        }

        if(i >= dims - 1) {
            throw ts::Exception("input parameters dims invalid");
        }

        int new_height = (int)stack.index(1)->sync(memory_device()).data<int>()[i];
        int new_width  = (int)stack.index(1)->sync(memory_device()).data<int>()[i+1];
        int old_height = (int)stack.index(0)->sizes()[i];
        int old_width  = (int)stack.index(0)->sizes()[i+1];

        std::vector<ts::Tensor::Prototype> output;
        infer(stack, output);

        ts::Shape shape(output[0].sizes());
        //ts::Shape shape(stack.index(0)->sizes());
        //shape[i] = new_height;
        //shape[i+1] = new_width;

        int ntotalbuffer = 0;
        int batchs,channels;
        batchs = channels = 0;
        bool ishwc = true;
        if(dims == 2) {
           batchs = channels = 1;
        }else if(dims == 3) {
           if(i== 0) {
               batchs = 1;
               channels = shape[2];
           }else if(i==1) {
               batchs = shape[0];
               channels = 1;
           }

        }else if(dims == 4) {
           if(i==0) {
              throw ts::Exception("input parameters dims invalid");
           }else if(i==1) {
              batchs = shape[0];
              channels = shape[3];
           }else if(i==2) {
              batchs = shape[0];
              channels = shape[1];
              ishwc = false;
           }
        }

        if(batchs <= 0 || channels <= 0) {
            throw ts::Exception("input parameters dims invalid");
        }

        //int imagetype = ts::Resize2d::getimagetype(stack.index(0)->dtype(), channels);
        //ntotalbuffer = batchs * channels * new_height * new_width;
        //std::cout << "channels:" << channels << ",batchs:" << batchs << ",total:" << ntotalbuffer << std::endl;

        //std::vector<ts::Tensor::Prototype> output;
        //output.resize(1);
        //ts::Tensor::Prototype prototype(ts::UINT8,shape);
        //ts::Tensor::Prototype prototype(stack.index(0)->dtype(),shape);
        //output[0] = prototype;
        //infer(stack, output);
        stack.push(output[0], memory_device());

        int newstep = channels * new_height * new_width;
        int oldstep = channels * old_height * old_width;

        auto &tensor = *stack.index(-1);
        /*
        int ntype = cv::INTER_LINEAR;
        if(has("type"))
        {
           std::cout << "type:" << get("type").data<int>()[0] << std::endl;
           if(get("type").data<int>()[0] != (int)linear)
               ntype = cv::INTER_CUBIC;
        }
        */
        for(int k=0; k<batchs; k++) {

           //psrc = stack.index(0)->data<unsigned char>() + k * oldstep;
           //psrc = stack.index(0)->data<float>() + k * oldstep;
           //printf("src:%x\n",psrc);

           //pdst = sum.data<unsigned char>() + k * newstep;
           //::cv::Mat srcimage(old_height, old_width,CV_8UC3,psrc);
           //pdst = sum.data<float>() + k * newstep;

           if(type_len == 1){
               resize<unsigned char>(stack, tensor, old_height,old_width,new_height,new_width,
                                     k * oldstep, k * newstep, newstep, channels, ishwc);
           }else if(type_len == 2) {
               resize<unsigned short>(stack, tensor, old_height,old_width,new_height,new_width,
                                     k * oldstep, k * newstep,  newstep, channels, ishwc);
           }else if(type_len == 4) {
               resize<float>(stack, tensor, old_height,old_width,new_height,new_width,
                                     k * oldstep, k * newstep, newstep, channels, ishwc);
           }else if(type_len == 8) {
               resize<double>(stack, tensor, old_height,old_width,new_height,new_width,
                                     k * oldstep, k * newstep, newstep, channels, ishwc);
           }

        }

        return 1;
    }

    virtual int infer(ts::Stack &stack, std::vector<ts::Tensor::Prototype> &output) {

        if(stack.size() != 2 || stack.index(0)->dims() < 2 || stack.index(0)->dims() != stack.index(1)->count()) {
            throw ts::Exception("input parameters is invalid");
        }

        int type_len = ts::type_bytes(stack.index(0)->dtype());

        if(stack.index(1)->dtype() != ts::INT32) {
            throw ts::Exception("input parameters 1 only support INT32 type");
        }

        int dims = stack.index(0)->dims();
        int i=0;
        for(i=0; i< dims; i++) {
            if((int)(stack.index(1)->sync(memory_device()).data<int>()[i]) > 0)
                break;
        }

        if(i >= dims - 1) {
            throw ts::Exception("input parameters dims invalid");
        }

        int new_height = (int)stack.index(1)->sync(memory_device()).data<int>()[i];
        int new_width  = (int)stack.index(1)->sync(memory_device()).data<int>()[i+1];
        //int old_height = (int)stack.index(0)->sizes()[i];
        //int old_width  = (int)stack.index(0)->sizes()[i+1];

        ts::Shape shape(stack.index(0)->sizes());

        shape[i] = new_height;
        shape[i+1] = new_width;

        output.resize(1);
        output[0] = ts::Tensor::Prototype(stack.index(0)->dtype(),shape);
        return 1;
    }

private:
template<typename T>
    void resize(ts::Stack &stack, ts::Tensor &tensor, int old_height, int old_width, 
           int new_height,int new_width, unsigned int oldstep, unsigned int newstep, unsigned int step, int channels, bool ishwc) {

           T*  psrc = stack.index(0)->sync(memory_device()).data<T>() + oldstep;;
           T*  pdst = tensor.sync(memory_device()).data<T>() + newstep;;

           int ntype = 0;
           if(has("type"))
           {
              std::cout << "type:" << get("type").sync(memory_device()).data<int>()[0] << std::endl;
              if(get("type").sync(memory_device()).data<int>()[0] != (int)linear)
                 ntype = 1;
           }
          
           if(ntype == 0) {
              ResizeImageLinear(psrc, old_width, old_height, channels, pdst,new_width,new_height, ishwc);
           }else {

              ResizeImageCubic(psrc, old_width, old_height, channels, pdst,new_width,new_height, ishwc);
           } 
           //::cv::Mat srcimage(old_height, old_width,imagetype,psrc);
           ////imwrite("/tmp/kkk3.png",srcimage);
           //::cv::Mat dstimage;// = srcimage;
           //::cv::resize(srcimage, dstimage, cv::Size(new_height, new_width),0,0, type);
           //::memcpy(pdst, dstimage.data, step * sizeof(T));
    }
};

}

#endif
