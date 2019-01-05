//
// Created by tang on 1/4/19.
//

#ifndef IVY_RAW_H
#define IVY_RAW_H

#include <memory>

namespace ivy {
namespace message {


struct Raw {
    std::shared_ptr<uint8_t> data;
    unsigned int length;

    explicit Raw(uint8_t* data);
};

}
}


#endif //IVY_RAW_H
