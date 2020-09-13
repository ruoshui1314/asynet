#ifndef ASYNET_LINK_LIST_H_
#define ASYNET_LINK_LIST_H_

template <typename T>
class LinkNode {
public:
    // LinkNode are self-referential as default.
    LinkNode() : previous_(this), next_(this) {}

    LinkNode(LinkNode<T>* previous, LinkNode<T>* next)
        : previous_(previous), next_(next) {}

    void insert_tail(LinkNode<T>* e) {
        this->next_ = e;
        this->previous_ = e->previous_;
        e->previous_->next_ = this;
        e->previous_ = this;
    }

    void insert_head(LinkNode<T>* e) {
        this->next_ = e->next_;
        this->previous_ = e;
        e->next_->previous_ = this;
        e->next_ = this;
    }

    void remove_from_list() {
        this->previous_->next_ = this->next_;
        this->next_->previous_ = this->previous_;
        this->next_ = this;
        this->previous_ = this;
    }

    LinkNode<T>* previous() const {
        return previous_;
    }

    LinkNode<T>* next() const {
        return next_;
    }

    // Cast from the node-type to the value type.
    const T* value() const {
        return static_cast<const T*>(this);
    }

    T* value() {
        return static_cast<T*>(this);
    }

private:
    LinkNode<T>* previous_;
    LinkNode<T>* next_;

};


template <typename T>
class LinkedList {
public:
    LinkedList() {}

    void append(LinkNode<T>* e) {
        root_.insert_tail(e);
    }

    LinkNode<T>* head() const {
        return root_.next();
    }

    LinkNode<T>* tail() const {
        return root_.previous();
    }

    const LinkNode<T>* end() const {
        return &root_;
    }

    bool empty() const {
        return head() == end();
    }

private:
    LinkNode<T> root_;

};

#endif
