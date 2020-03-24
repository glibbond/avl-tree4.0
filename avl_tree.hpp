#ifndef GB_AVL_TREE
#define GB_AVL_TREE

#include <iostream>
#include <cstdint>
#include <array>
#include <thread>
#include <tuple>
#include <stack>

namespace AVL{
    using int_t = int32_t;
    using uint_t = uint32_t;

    using direction_t = bool;
    constexpr direction_t left = false;
    constexpr direction_t right = true;
    int_t weight(direction_t dir){
        return (static_cast<int_t>(dir) << 1) - 1;
    }

    template<typename T>
    using stack_t = std::stack<T>;

    enum class iterator_dir: uint_t{
        forward = 0,
        reverse = 1,
    };

    enum class mask_t: int8_t{
        balance_factor_mask = 7,               // 00000111
        child_mask = 8,                        // 00001000
        //  left_child_mask == child_mask << 0 == 00001000
        // right_child_mask == child_mask << 1 == 00010000
        default_mask = 2,                      // 00000010
    };

    using ipair = std::pair<int_t, int_t>;

    constexpr int_t SC = 13; // in case the theory used here does not apply to every possible case
    constexpr std::array<std::array<std::pair<int_t, int_t>, 5>, 5> balance_factor_table{
                                    // 2          // 1          // 0          //-1          //-2
        std::array<ipair, 5>{ipair( 1, 0), ipair( 0, 0), ipair(-1, 1), ipair(-1, 2), ipair(SC,SC),}, //-2
        std::array<ipair, 5>{ipair(SC,SC), ipair( 1, 1), ipair( 0, 1), ipair( 0, 2), ipair(SC,SC),}, //-1
        std::array<ipair, 5>{ipair(SC,SC), ipair(SC,SC), ipair(SC,SC), ipair(SC,SC), ipair(SC,SC),}, // 0
        std::array<ipair, 5>{ipair(SC,SC), ipair( 0,-2), ipair( 0,-1), ipair(-1,-1), ipair(SC,SC),}, // 1
        std::array<ipair, 5>{ipair(SC,SC), ipair( 1,-2), ipair( 1,-1), ipair( 0, 0), ipair(-1, 0),}, // 2
    };

    const ipair& get_updated_balance_factors(int_t parent_bf, int_t child_bf){
        return balance_factor_table[parent_bf + 2][child_bf + 2];
    }

    template<typename T>
    class Tree{
        private:
        template<typename node_t, iterator_dir direction>
        class Iterator;

        class Node{
            private:
                friend class AVL::Tree<T>;
                template<typename node_t, iterator_dir direction>
                friend class AVL::Tree<T>::Iterator;
                T value_;
                Node* children_;
                int8_t bits_;
            public:
                int_t balance_factor() const { return (static_cast<int8_t>(mask_t::balance_factor_mask) & this->bits_) - 2; }
                void shift_balance_factor(int_t amount){ this->set_balance_factor(balance_factor() + amount); }
                void set_balance_factor(int8_t value){ this->bits_ = ((~static_cast<int8_t>(mask_t::balance_factor_mask)) & this->bits_) +
                                                                            (static_cast<int8_t>(mask_t::balance_factor_mask) &
                                                                            (static_cast<int8_t>(mask_t::default_mask) + value)); }
                void reset_balance_factor(){ this->bits_= ((~static_cast<int8_t>(mask_t::balance_factor_mask)) & this->bits_) +
                                                             static_cast<int8_t>(mask_t::default_mask); }

                bool has_child(direction_t dir) const { return (static_cast<int8_t>(mask_t::child_mask) << dir) & this->bits_; }
                bool has_any_children() const { return has_child(left) || has_child(right); }
                bool has_both_children() const { return has_child(left) && has_child(right); }
                void set_child(direction_t dir){   this->bits_ |=   static_cast<int8_t>(mask_t::child_mask) << dir; }
                void reset_child(direction_t dir){ this->bits_ &= ~(static_cast<int8_t>(mask_t::child_mask) << dir); }

                const T& value() const { return this->value_; }
                T& value(){ return this->value_; }

                template<typename U>
                void add_leaf(U&& value, direction_t dir);
                void remove_leaf(direction_t dir);

                Node& operator=(Node&& that);

                Node(): children_(nullptr), bits_(static_cast<int8_t>(mask_t::default_mask)){}
                Node(const T& X):       value_(X), children_(nullptr), bits_(static_cast<int8_t>(mask_t::default_mask)){}
                Node(T&& X): value_(std::move(X)), children_(nullptr), bits_(static_cast<int8_t>(mask_t::default_mask)){}
                Node(Node&& N): value_(std::move(N.value_)), children_(N.children_), bits_(N.bits_){
                    N.children_ = nullptr;
                    N.bits_ = static_cast<int8_t>(mask_t::default_mask);
                }
            private:
                void rotate(direction_t dir);
                void rotate(direction_t dir1, direction_t dir2);
                void fix();
                #ifdef GB_PRINT
                void print(){
                    system("clear");
                    this->recursive_print(40, 0, 1);
                    // getchar();
                }
                void recursive_print(uint32_t size, uint32_t offset, uint32_t level){
                    printf("\033[%d;%dH%d\n", level, 4 * offset + 2 * size, *reinterpret_cast<int*>(&this->value_));
                    printf("\033[%d;%dH%d %d %d\n", level + 1, 4 * offset + 2 * size - 3, has_child(0), balance_factor(), has_child(1));
                    if(this->has_child(left)) this->children_[left ].recursive_print(size/2, offset,          level + 4);
                    if(this->has_child(right))this->children_[right].recursive_print(size/2, offset + size/2, level + 4);
                }
                #endif
        };

        template<typename node_t>
        using position_t = stack_t<std::pair<node_t*, direction_t>>;

        private:
            template<typename node_t, iterator_dir direction>
            class Iterator{ // REDO FOR JUST ONE STACK CONSISTING OF PAIRS
                private:
                    friend class AVL::Tree<T>;
                    position_t<node_t> position_;
                public:
                    const Node*& top(){
                        return const_cast<const Node*&>(this->position_.top().first);
                    }
                    const Node& current_node() const { return *(this->position_.top().first); }
                    node_t& current_node(){ return *(this->position_.top().first); }
                    const direction_t& current_direction() const { return this->position_.top().second; }
                    direction_t& current_direction(){ return this->position_.top().second; }

                    auto& operator*(){ return (this->current_node()).value_; }

                    void increment(){
                        if(top() == nullptr) return;
                        if(current_node().has_any_children()){
                            direction_t dir = !(current_node().has_child(left));
                            current_direction() = dir;
                            position_.push(std::pair<node_t*, direction_t>(current_node().children_ + dir, 0));
                        }
                        else do{
                            position_.pop();
                            if(top() == nullptr) return;
                            if(current_node().has_child(right) && current_direction() != right){
                                current_direction() = right;
                                position_.push(std::pair<node_t*, direction_t>(current_node().children_ + right, 0));
                                return;
                            }
                        } while(true);
                    }

                    void decrement(){
                        if(top() == nullptr) return;
                        position_.pop();
                        if(top() == nullptr) return;
                        if(current_node().has_child(left) && current_direction() != left){
                            current_direction() = left;
                            position_.push(std::pair<node_t*, direction_t>(current_node().children_ + left, 0));
                        } else return;
                        while(current_node().has_any_children()){
                            direction_t dir = current_node().has_child(right);
                            current_direction() = dir;
                            position_.push(std::pair<node_t*, direction_t>(current_node().children_ + dir, 0));
                        }
                    }

                    Iterator& operator++(){
                        if constexpr(direction == iterator_dir::forward) this->increment();
                        else                               this->decrement();
                        return *this;
                    }
                    Iterator operator++(int){
                        Iterator result = *this;
                        ++(*this);
                        return result;
                    }
                    Iterator& operator--(){
                        if constexpr(direction == iterator_dir::forward) this->decrement();
                        else                               this->increment();
                        return *this;
                    }
                    Iterator operator--(int){
                        Iterator result = *this;
                        --(*this);
                        return result;
                    }

                    Iterator& operator=(const Iterator& that){
                        this->position_ = that.position_;
                        return *this;
                    }
                    Iterator& operator=(Iterator&& that){
                        this->position_ = std::move(that.position_);
                        that = Iterator();
                        return *this;
                    }

                    bool operator==(const Iterator& that) const { return this->position_.top().first == that.position_.top().first; }
                    bool operator!=(const Iterator& that) const { return !(*this == that); }
                    node_t& node(){ return *(this->position_.top().first); }

                    Iterator(){ position_.push(std::pair<Node*, direction_t>(nullptr, 0)); }
                    Iterator(const position_t<node_t>& position): position_(position){}
                    Iterator(position_t<node_t>&& position): position_(std::move(position)){}
                    Iterator(const Iterator& that): position_(that.position_){}
                    Iterator(Iterator&& that): position_(std::move(that.position_)){ that = Iterator(); }
                    position_t<node_t>& position(){ return this->position_; }
                    const position_t<node_t>& position() const { return this->position_; }
            };
        public:
            const static std::pair<stack_t<Node*>, stack_t<direction_t>> end_;

            using forward_iterator_t = Iterator<Node, iterator_dir::forward>;
            using forward_const_iterator_t = Iterator<const Node, iterator_dir::forward>;
            using reverse_iterator_t = Iterator<Node, iterator_dir::reverse>;
            using reverse_const_iterator_t = Iterator<const Node, iterator_dir::reverse>;

            forward_iterator_t begin();
            forward_iterator_t end();
            forward_const_iterator_t cbegin() const;
            forward_const_iterator_t cend() const;
            reverse_iterator_t rbegin();
            reverse_iterator_t rend();
            reverse_const_iterator_t crbegin() const;
            reverse_const_iterator_t crend() const;
        private:
            Node root_;
            size_t size_;

            template<typename U>
            void add_priv(U&& X);

            template<typename node_t>
            position_t<node_t> find_spot(const T& X) const;

            template<typename node_t>
            position_t<node_t> find_priv(const T& X) const;

            template<typename node_t>
            position_t<node_t> find_farthest(direction_t dir) const;
        public:
            bool empty() const { return size_ == 0; }
            size_t size() const { return this->size_; }
            size_t height() const;

            void add(const T& X){ this->add_priv(X); }
            void add(T&& X){ this->add_priv(std::move(X)); }

            bool remove(position_t<Node> position);

            template<iterator_dir direction>
            bool remove(Iterator<Node, direction>& i){
                return remove(i.position());
            }
            bool remove(const T& X){
                return remove(this->find(X));
            }

            position_t<Node> find(const T& X){ return find_priv<Node>(X); }
            position_t<const Node> find(const T& X) const { return find_priv<const Node>(X); }
            position_t<Node> find_farthest(direction_t dir){ return find_farthest<Node>(); }
            position_t<const Node> find_farthest(direction_t dir) const { return find_farthest<const Node>(); }
            template<typename node_t>
            static position_t<node_t> find_farthest(direction_t dir, position_t<node_t> position);

            Tree& operator=(const Tree& that);
            Tree& operator=(Tree&& that);

            #ifdef GB_PRINT
                void print(){
                    if(!this->empty())
                        this->root_.print();
                    else
                        system("clear");
                    printf("\033[%d;%dH\nSIZE: %d\n\n", 1, 0, static_cast<int>(size_));
                }
            #endif
            Tree(): size_(0){}
            Tree(const Tree& that){
                *this = that;
            }
            Tree(Tree&& that): root_(std::move(that.root_)), size_(that.size_){
                that.size_ = 0;
            }
            Tree(std::initializer_list<T> list): size_(0){
                for(auto& element : list){
                    this->add(element);
                }
            }
            Tree(const T& X): root_(Node(X)), size_(0){}
            Tree(T&& X): root_(Node(std::move(X))), size_(0){}
            Tree(Node&& that): root_(std::move(that)){
                size_ = 0;
                for(const auto& i: *this){
                    ++size_;
                }
            }
            ~Tree(){
                for(auto ri = this->rbegin(); ri != this->rend(); ++ri){
                    ri.current_node().remove_leaf(left);
                    ri.current_node().remove_leaf(right);
                }
            }
    };

template<typename T>
template<typename U>
void Tree<T>::Node::add_leaf(U&& value, direction_t dir){
    if(this->children_ == nullptr){
        this->children_ = reinterpret_cast<Node*>(new uint8_t[sizeof(Node) * 2]);
    }
    if(!has_child(dir)){
        new (this->children_ + dir) Node (std::forward<U>(value));
        set_child(dir);
        shift_balance_factor(weight(dir));
    }
}

template<typename T>
void Tree<T>::Node::remove_leaf(direction_t dir){
    if(has_child(dir)){
        this->children_[dir].value_.~T();
        this->children_[dir].bits_ = 0;
        reset_child(dir);
        shift_balance_factor(-weight(dir));
    }
    if(!has_any_children() && children_ != nullptr){
        delete[] reinterpret_cast<uint8_t*>(children_);
        children_ = nullptr;
    }
}

template<typename T>
typename Tree<T>::Node& Tree<T>::Node::operator=(typename Tree<T>::Node&& that){
    this->value_ = std::move(that.value_);
    this->children_ = that.children_;
    this->bits_ = that.bits_;

    that.children_ = nullptr;
    that.bits_ = static_cast<int8_t>(mask_t::default_mask);

    return *this;
}

template<typename T>
void Tree<T>::Node::rotate(direction_t dir){
    int_t ABF = 0;
    int_t BBF = 0;
    Node& A = *this;
    ABF = A.balance_factor();
    {
        Node& B = this->children_[!dir];
        BBF = B.balance_factor();
        if(B.has_child(dir)){
            Node& C = B.children_[dir];
            std::swap(A, B);
            std::swap(C, B);
        }
        else if(B.has_child(!dir)){
            Node& C = B.children_[dir];
            std::swap(A, B);
            new (&C) Node (std::move(B));
            B.~Node();
            C.reset_child(!dir);
            if(!C.has_any_children()){
                delete reinterpret_cast<uint8_t*>(C.children_);
                C.children_ = nullptr;
            }
            A.set_child(dir);
        } else {
            new (A.children_ + dir) Node (std::move(A.value_));
            A.value_ = std::move(B.value_);
            A.set_child(dir);
            A.reset_child(!dir);
            A.children_[!dir].~Node();
        }
    }
    Node& E = A.children_[dir];
    const ipair& updated_balance_factors = get_updated_balance_factors(ABF, BBF);
    A.set_balance_factor(updated_balance_factors.second);
    E.set_balance_factor(updated_balance_factors.first);
}

template<typename T>
void Tree<T>::Node::rotate(direction_t dir1, direction_t dir2){
    if(dir1 != dir2){
        this->children_[!dir2].rotate(dir1);
    }
    this->rotate(dir2);
}

template<typename T>
void Tree<T>::Node::fix(){
    Node& Parent = *this;
    direction_t dir2 = !(Parent.balance_factor() > 0);
    Node& Child = Parent.children_[!dir2];
    direction_t dir1 = static_cast<bool>(Child.balance_factor()) * (((-Child.balance_factor()) + 1) >> 1)
                    + (!static_cast<bool>(Child.balance_factor())) * dir2;
    Parent.rotate(dir1, dir2);
}




template<typename T>
typename Tree<T>::forward_iterator_t Tree<T>::begin(){
    forward_iterator_t iterator = forward_iterator_t();
    iterator.position_.push(std::pair<Node*, direction_t>(&this->root_, 0)); 
    return iterator;
}

template<typename T>
typename Tree<T>::forward_iterator_t Tree<T>::end(){ return forward_iterator_t(); }

template<typename T>
typename Tree<T>::forward_const_iterator_t Tree<T>::cbegin() const {
    forward_const_iterator_t iterator = forward_const_iterator_t();
    iterator.position_.push(std::pair<const Node*, direction_t>(&this->root_, 0)); 
    return iterator;
}

template<typename T>
typename Tree<T>::forward_const_iterator_t Tree<T>::cend() const { return forward_const_iterator_t(); }

template<typename T>
typename Tree<T>::reverse_iterator_t Tree<T>::rbegin(){
    reverse_iterator_t iterator = reverse_iterator_t();
    iterator.position_.push(std::pair<Node*, direction_t>(&this->root_, right));
    iterator.position_ = find_farthest(right, iterator.position_); 
    iterator.position_ = find_farthest(left,  iterator.position_);
    return iterator; 
}

template<typename T>
typename Tree<T>::reverse_iterator_t Tree<T>::rend(){ return reverse_iterator_t(); }

template<typename T>
typename Tree<T>::reverse_const_iterator_t Tree<T>::crbegin() const {
    reverse_const_iterator_t iterator = reverse_const_iterator_t();
    iterator.position_.push(std::pair<const Node*, direction_t>(&this->root_, right));
    iterator.position_ = find_farthest(right, iterator.position_); 
    iterator.position_ = find_farthest(left,  iterator.position_);
    return iterator; 
}

template<typename T>
typename Tree<T>::reverse_const_iterator_t Tree<T>::crend() const { return reverse_const_iterator_t(); }

template<typename T>
template<typename U>
void Tree<T>::add_priv(U&& X){
    ++size_;
    if(size_ == 1){
        root_.value_ = std::forward<U>(X);
        return;
    }
    position_t<Node> position = find_spot<Node>(X);
    position.top().first->add_leaf(std::forward<U>(X), position.top().second);
    if(position.top().first->balance_factor() == 0){
        return;
    }
    position.pop();
        while(position.top().first != nullptr){
        int_t new_bf = position.top().first->balance_factor() + (static_cast<int_t>(position.top().second) << 1) - 1;
        position.top().first->set_balance_factor(new_bf);
        if(new_bf == 2 || new_bf == -2){
            position.top().first->fix();
        }
        if(position.top().first->balance_factor() == 0){
            break;
        }
        position.pop();
    }
}

template<typename T>
template<typename node_t>
Tree<T>::position_t<node_t> Tree<T>::find_spot(const T& X) const{
    position_t<node_t> position;
    position.push(std::pair<node_t*, direction_t>(nullptr, 0));
    if(this->empty()){
        return position;
    }
    node_t* current_ptr = const_cast<node_t*>(&this->root_);
    direction_t dir = 0;
    position.push(std::pair<node_t*, direction_t>(current_ptr, 0));
    while(current_ptr->has_child(dir = (X > current_ptr->value_))){
        position.top().second = dir;
        current_ptr = current_ptr->children_ + dir;
        position.push(std::pair<node_t*, direction_t>(current_ptr, 0));
    }
    position.top().second = dir;
    return position;
}

template<typename T>
template<typename node_t>
Tree<T>::position_t<node_t> Tree<T>::find_priv(const T& X) const {
    position_t<node_t> position;
    position.push(std::pair<node_t*, direction_t>(nullptr, 0));
    if(this->empty()){
        return position;
    }
    node_t* current_ptr = const_cast<node_t*>(&this->root_);
    direction_t dir = 0;
    while(current_ptr->has_child(dir = X > current_ptr->value_) && current_ptr->value_ != X){
        position.push(std::pair<node_t*, direction_t>(current_ptr, dir));
        current_ptr = current_ptr->children_ + dir;
    }
    if(current_ptr->value_ == X){
        position.push(std::pair<node_t*, direction_t>(current_ptr, 0));
    } else {
        position.push(std::pair<node_t*, direction_t>(nullptr, 0));
    }
    return position;
}

template<typename T>
template<typename node_t>
Tree<T>::position_t<node_t> Tree<T>::find_farthest(direction_t dir) const {
    position_t<node_t> position;
    position.push(std::pair<node_t*, direction_t>(nullptr, 0));
    if(this->empty()){
        return position;
    }
    node_t* current_ptr = const_cast<node_t*>(&this->root);
    while(current_ptr->has_child(dir)){
        position.push(std::pair<node_t*, direction_t>(current_ptr, dir));
    }
    position.push(std::pair<node_t*, direction_t>(current_ptr, 0));
    return position;
}

template<typename T>
size_t Tree<T>::height() const {
    uint_t height_ = 0;
    if(size == 0) return height_;
    const Node* current_node = this;
    direction_t dir = 0;
    while(current_node->has_child( dir = (current_node->balance_factor() + 1) >> 1 )){
        current_node = current_node->children_ + dir;
        ++height;
        }
    return height;
}

template<typename T>
bool Tree<T>::remove(Tree<T>::position_t<Node> position){
    if(position.top().first == nullptr) return false;
    if(position.top().first == &root_ && size_ == 1){
        size_ = 0;
        root_ = Node();
        return true;
    }
    Node& target = *(position.top().first);
    if(position.top().first->has_any_children()){
        direction_t dir = !position.top().first->has_child(left);
        position.top().second = dir;
        position.push(std::pair<Node*, direction_t>(position.top().first->children_ + dir, !dir));
        position = find_farthest(!dir, position);
        target.value_ = std::move(position.top().first->value_);
        if(position.top().first->has_child(dir)){
            Node temp = std::move(position.top().first->children_[dir]);
            position.top().first->remove_leaf(dir);
            *(position.top().first) = std::move(temp);
            position.pop();
            position.top().first->shift_balance_factor((static_cast<int_t>(!position.top().second) << 1) - 1);
        } else {
            position.pop();
            position.top().first->remove_leaf(position.top().second);
        }
    } else {
        position.pop();
        position.top().first->remove_leaf(position.top().second);
    }
    while(true){
        if(position.top().first->balance_factor() == 2 || position.top().first->balance_factor() == -2){
            position.top().first->fix();
            if(position.top().first->balance_factor() != 0){
                break;
            }
        } else if(position.top().first->balance_factor() == (static_cast<int_t>(!position.top().second) << 1) - 1){
            break;
        }
        position.pop();
        if(position.top().first == nullptr){
            break;
        }
        position.top().first->shift_balance_factor((static_cast<int_t>(!position.top().second) << 1) - 1);
    }
    --size_;
    return true;
}

template<typename T>
template<typename node_t>
Tree<T>::position_t<node_t> Tree<T>::find_farthest(direction_t dir, Tree<T>::position_t<node_t> position){
    if(position.top().first == nullptr){
        return position;
    }
    position.top().second = dir;
    node_t* current_ptr = position.top().first;
    while(current_ptr->has_child(dir)){
        current_ptr = current_ptr->children_ + dir;
        position.push(std::pair<node_t*, direction_t>(current_ptr, dir));
    }
    return position;
}

template<typename T>
Tree<T>& Tree<T>::operator=(const Tree& that){
    position_t<Node> this_path;
    this_path.push(std::pair<Node*, direction_t>(nullptr, 0));
    this_path.push(std::pair<Node*, direction_t>(&this->root_, 0));
    position_t<const Node> that_path;
    that_path.push(std::pair<const Node*, direction_t>(nullptr, 0));
    that_path.push(std::pair<const Node*, direction_t>(&that.root_, 0));
    while(that_path.top().first != nullptr){
        this_path.top().first->value_ = that_path.top().first->value_;
        std::cout << this_path.top().first->value_ << "\n";
        if(that_path.top().first->has_any_children()){
            direction_t dir = !(that_path.top().first->has_child(left));
            this_path.top().first->add_leaf(T(), dir);

            const_cast<direction_t&>(that_path.top().second) = dir;
            const_cast<direction_t&>(this_path.top().second) = dir;

            that_path.push(std::pair<const Node*, direction_t>(that_path.top().first->children_ + dir, 0));
            this_path.push(std::pair<Node*, direction_t>(this_path.top().first->children_ + dir, 0));
        }
        else do{
            that_path.pop();
            this_path.pop();
            if(that_path.top().first == nullptr) break;
            if(that_path.top().first->has_child(right) && that_path.top().second != right){
                this_path.top().first->add_leaf(T(), right);
                const_cast<direction_t&>(that_path.top().second) = right;
                const_cast<direction_t&>(this_path.top().second) = right;
                that_path.push(std::pair<const  Node*, direction_t>(that_path.top().first->children_ + right, 0));
                this_path.push(std::pair<Node*, direction_t>(this_path.top().first->children_ + right, 0));
                break;
            }
        } while(true);
    }
    return *this;
}

template<typename T>
Tree<T>& Tree<T>::operator=(Tree&& that){
    this->root_ = std::move(that.root_);
    this->size_ = that.size_;
    that.size_ = 0;
    return *this;
}
}

#endif