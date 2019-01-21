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
    uint64_t id;


    explicit Raw(uint8_t* data, uint64_t id);
};

}
}


#endif //IVY_RAW_H
