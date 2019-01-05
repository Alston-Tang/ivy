//
// Created by tang on 1/4/19.
//

#include "raw.h"

namespace ivy {
namespace message {


// https://stackoverflow.com/questions/13061979/shared-ptr-to-an-array-should-it-be-used
template< typename T >
struct array_deleter
{
    void operator ()( T const * p)
    {
        delete[] p;
    }
};


Raw::Raw(uint8_t *data) {
    this->data = std::shared_ptr<uint8_t>(data, array_deleter<uint8_t>());
    this->length = 0;
}

}
}