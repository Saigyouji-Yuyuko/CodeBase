#include "index.hpp"

namespace CodeBase {

template<typename KeyType, typename ValueType>
class BplusNode {
public:
    BplusNode(Page *page) : page_(page) {}
    enum BplusNodeType { Parent = 0, Leaf } NodeType;
    bool isLeaf() {
        return NodeType == BplusNodeType::Leaf;
    }
    Page *page_;
};

template<typename KeyType, typename ValueType>
class BplusParentNode : public BplusNode<KeyType, ValueType> {
public:
    using BaseType = BplusNode<KeyType, ValueType>;
    BplusParentNode(Page *page) : BaseType(page) {
        this->NodeType = BaseType::BplusNodeType::Parent;
    }
};

template<typename KeyType, typename ValueType>
class BplusLeafNode : public BplusNode<KeyType, ValueType> {
public:
    using BaseType = BplusNode<KeyType, ValueType>;
    BplusLeafNode(Page *page) : BaseType(page) {
        this->NodeType = BaseType::BplusNodeType::Leaf;
    }
};


template<typename KeyType, typename ValueType, typename Context, typename Compare>
class BplusTreeIndexIterator;

template<typename KeyType, typename ValueType, typename Context, typename Compare = std::less<KeyType>>
class BplusTreeIndex : public Compare {
public:
    using Iterator = BplusTreeIndexIterator<KeyType, ValueType, Context, Compare>;
    using ConstIterator = const Iterator;
    static_assert(IndexIterator<Iterator>);
    static_assert(ConstIndexIterator<ConstIterator>);

    ErrorCode     Insert(Context &context, const KeyType &key, const ValueType &value);
    ErrorCode     Delete(Context &context, const KeyType &key);
    ErrorCode     Update(Context &context, const KeyType &key, const ValueType &value);
    Iterator      Find(Context &context, const KeyType &key);
    ConstIterator Find(Context &context, const KeyType &key) const;
    Iterator      Begin(Context &context);
    Iterator      End(Context &context);
    ConstIterator Begin(Context &context) const;
    ConstIterator End(Context &context) const;

private:
    Page root_;
};


}// namespace CodeBase