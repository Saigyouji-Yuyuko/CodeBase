#include "index.hpp"
#include "utils/error.hpp"
#include <cstdint>

namespace CodeBase {

template<typename T>
concept ValueType = requires(T t) {
                        { t.Load((const char *){}) } -> std::same_as<ErrorCode>;
                        { t.Save((char *){}) } -> std::same_as<ErrorCode>;
                        { t.LayoutSize() } -> std::same_as<size_t>;
                    };

template<typename T>
concept KeyType = ValueType<T> && requires(T t) {
                                      { t.operator<=>(T{}) } -> std::same_as<int>;
                                  };

struct BplusTreeIndexLayoutBase {
    uint64_t magic;
    uint64_t crc;
    uint64_t type;
    uint64_t version;
    uint64_t size;
};

void InitBplusTreeIndexLayoutBase(BplusTreeIndexLayoutBase *layout) {
    layout->magic = 0x12345678;
    layout->crc = 0;
    layout->type = 0;
    layout->version = 0;
    layout->size = 0;
}

template<KeyType Key, ValueType Value>
class BplusNode {
public:
    BplusNode() = default;
    enum BplusNodeType { Parent = 0, Leaf } NodeType;
    bool isLeaf() {
        return NodeType == BplusNodeType::Leaf;
    }
    static std::pair<BplusNode, ErrorCode> *MakeBlusPage(Page *);

    Page *page;
};

template<KeyType Key, ValueType Value>
class BplusParentNode final : public BplusNode<Key, Value> {
public:
    using BaseType = BplusNode<Key, Value>;
    BplusParentNode(Page *page) {
        this->NodeType = BaseType::BplusNodeType::Parent;
    }
    static void Format(Page *page);


    std::vector<BaseType *> Childrens;
};

template<KeyType Key, ValueType Value>
class BplusLeafNode final : public BplusNode<Key, Value> {
public:
    using BaseType = BplusNode<Key, Value>;
    BplusLeafNode(Page *page) {
        this->NodeType = BaseType::BplusNodeType::Leaf;
    }

    void Insert2Leaf(const Key &key, const Value &value);

    static void Format(Page *page);
};


template<KeyType Key, ValueType Value, typename Compare>
class BplusTreeIndexIterator;

template<KeyType Key, ValueType Value, typename Compare = std::less<Key>>
class BplusTreeIndex : public Compare {
public:
    using Iterator = BplusTreeIndexIterator<Key, Value, Compare>;
    using ConstIterator = const Iterator;

    static_assert(IndexIterator<Iterator>);

    Task<ErrorCode>     Insert(IContext *context, const Key &key, const Value &value);
    Task<ErrorCode>     Delete(IContext *context, const Key &key);
    Task<ErrorCode>     Update(IContext *context, const Key &key, const Value &value);
    Task<Iterator>      Find(IContext *context, const Key &key);
    Task<ConstIterator> Find(IContext *context, const Key &key) const;
    Task<Iterator>      Begin(IContext *context);
    Task<Iterator>      End(IContext *context);
    Task<ConstIterator> Begin(IContext *context) const;
    Task<ConstIterator> End(IContext *context) const;

private:
    BplusNode<Key, Value> *root_;
};

template<KeyType Key, ValueType Value>
std::pair<BplusNode<Key, Value>, ErrorCode> *BplusNode<Key, Value>::MakeBlusPage(Page *page) {
    BplusTreeIndexLayoutBase *layout = (BplusTreeIndexLayoutBase *) page->data;
    if (layout->magic != 0x12345678) {
        return {nullptr, Error::Fail};
    }
    if ((layout->type & 0) == 0) {
        return {new BplusParentNode<Key, Value>(page), Error::Success};
    } else if ((layout->type & 1) == 1) {
        return {new BplusLeafNode<Key, Value>(page), Error::Success};
    } else {
        return {nullptr, Error::Fail};
    }
}

template<KeyType Key, ValueType Value, typename Compare>
Task<ErrorCode> BplusTreeIndex<Key, Value, Compare>::Insert(IContext *context, const Key &key, const Value &value) {
    if (this->root_ == nullptr) {
        Page *page = nullptr;
        auto  err = co_await context->GetStorage()->AllocPage(&page);
        if (err != Error::Success) {
            co_return err;
        }
        BplusLeafNode<Key, Value>::Format(page);
        this->root_ = BplusNode<Key, Value>::MakeBlusPage(page);

        //update root page
        co_await context->GetStorage()->MarkDirty(page->GetPageID());
        co_return Error::Success;
    } else {
    }
}


}// namespace CodeBase