#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

struct OrderRefLut
{
    std::vector<uint32_t> m_orderRefLut;

    uint32_t orderRefLookup(size_t orderRef) const {
        if (orderRef >= m_orderRefLut.size() || orderRef < 0) [[unlikely]] {
            return (uint32_t)-1;
        }
        return m_orderRefLut[orderRef];
    }

    void setOrderRef(size_t orderRef, uint32_t userLocalID) {
        if (orderRef < 0) [[unlikely]] {
            return;
        }
        if (orderRef >= m_orderRefLut.size()) {
            if (orderRef >= m_orderRefLut.max_size()) [[unlikely]] {
                return;
            }
            m_orderRefLut.resize(orderRef + 1, -1);
        }
        m_orderRefLut[orderRef] = userLocalID;
    }
};
