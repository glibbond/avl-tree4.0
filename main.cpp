#define GB_PRINT
#include <avl_tree.hpp>
#include <bitset>
#include <vector>
#include <array>
#include <integer.h>
#include <chrono>
#include <thread>
#include <cmath>

template<typename T>
void node_info(typename AVL::Tree<T>::Node const & node){
    std::cout
        << "VALUE: " << node.value_ << "\n"
        << "HAS LEFT CHILD:  " << node.has_child(0) << "\n"
        << "HAS RIGHT CHILD: " << node.has_child(1) << "\n"
        << "BALANCE FACTOR:  " << node.balance_factor() << "\n"
        << "\n"
        ;
}

void ipair_info(const AVL::ipair& pair){
    std::cout << pair.first << "\t" << pair.second << "\n";
}

void rotate_test_11(bool dir){
    AVL::Tree<int>::Node node(10);
    node.add_leaf(5, !dir);
    node.add_leaf(15, dir);
    node.children_[dir].add_leaf(12, !dir);
    node.children_[dir].add_leaf(17, dir);
    node.set_balance_factor((static_cast<int>(dir) << 1) - 1);
    node.print();
    node.rotate(!dir);
    node.print();
}

void rotate_test_12(bool dir){
    AVL::Tree<int>::Node node(10);
    node.add_leaf(5, !dir);
    node.add_leaf(15, dir);
    node.children_[dir].add_leaf(12, !dir);
    node.children_[dir].add_leaf(17, dir);
    node.children_[dir].children_[dir].add_leaf(20, dir);
    node.set_balance_factor(2 * ((static_cast<int>(dir) << 1) - 1));
    node.children_[dir].set_balance_factor(((static_cast<int>(dir) << 1) - 1));
    node.print();
    node.fix();
    node.print();
}

void rotate_test_13(bool dir){
    AVL::Tree<int>::Node node(10);
    node.add_leaf(5, !dir);
    node.add_leaf(15, dir);
    node.children_[dir].add_leaf(12, !dir);
    node.children_[dir].add_leaf(17, dir);
    node.children_[dir].children_[!dir].add_leaf(14, !dir);
    node.set_balance_factor(2 * ((static_cast<int>(dir) << 1) - 1));
    node.children_[dir].set_balance_factor(((static_cast<int>(!dir) << 1) - 1));
    node.print();
    node.fix();
    node.print();
}

void rotate_test_2(bool dir){
    AVL::Tree<int>::Node node(10);
    node.add_leaf(5, !dir);
    node.add_leaf(15, dir);
    node.children_[dir].add_leaf(17, dir);
    node.set_balance_factor((static_cast<int>(dir) << 1) - 1);
    node.print();
    node.rotate(!dir);
    node.print();
}

int main(){
    // std::cout <<  std::bitset<sizeof(uint8_t) * 8>( 7 ) << "\t"
    //           <<  std::bitset<sizeof(uint8_t) * 8>( 8 ) << "\t"
    //           << (std::bitset<sizeof(uint8_t) * 8>( 8 ) << 1) << "\n";
    // AVL::Tree<int> tree;
    // AVL::Tree<int>::forward_iterator_t fit = tree.end();

    // std::cout << sizeof(std::vector<char>) << "\t" << sizeof(std::stack<long double>) << "\n";
    // std::cout << sizeof(fit) << "\t" << fit.path_.size() << "\t" << fit.directions_.size() << "\n";

    // int shift = 2;
    // // int shift = (sizeof(int) * 8);
    // std::cout << std::bitset<sizeof(int) * 8>(-8 >> shift) << "\n";
    // std::cout << std::bitset<sizeof(int) * 8>( 8 >> shift) << "\n";

    // std::vector<int> vector;
    // vector.reserve(100000);
    // vector.push_back(10);
    // auto begin = vector.begin();
    // std::cout << ++*begin << "\t" << vector.capacity() << "\n";
    // vector.reserve(1000);
    // std::cout << ++*begin << "\t" << vector.capacity() << "\n";
    // std::cout << "END\n";

    // std::stack<int> stack;
    // stack.push(10);
    // const int& ref1 = stack.top();
    // int& ref2 = stack.top();
    // int const& ref3 = stack.top();

    // std::cout << ref1 << "\t" << ref2 << "\t" << ref3 << "\n";

    // ref2 = 4;

    // std::cout << ref1 << "\t" << ref2 << "\t" << ref3 << "\n";

    // // ref3 = 6;

    auto list = {2, 5, 3, -8, 3, 9, -10, 2, 6, 22, 0, -2, -2, 3, -3, 45, 22, -4, -9, 5, 0, -2, -44, -2, -76, 3, 4, 2, 5, 3, -8, 3, 9, -10, 2, 6, 22, 0, -2, -2, 3, -3, 45, 22, -4, -9, 5, 0, -2, -44, -2, -76, 3, 4};
    AVL::Tree<int> tree;
    double var = 1;
    for(auto& i: list){
        tree.add(i);
        tree.print();
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1200 / log2(var))));
        var *= 2;
    }
    AVL::Tree<int> tree2(tree);

    for(auto i = tree.cbegin(); i != tree.cend(); ++i){
        std::cout << *i << " ";
    }
    std::cout << "\n";
    for(auto ri = tree.rbegin(); ri != tree.rend(); ++ri){
        std::cout << *ri << " ";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    for(auto& i: list){
        tree.remove(i);
        tree.print();
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1200 / log2(var))));
        var /= 2;
    }
    std::cout << "\n";
    // getchar();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // tree.print();
    // for(const auto& el: list){
    //     tree.remove(el);
    //     tree.print();
    // }
    // std::cout << tree.empty() << "\n";

    // std::stack<int> stack;
    // stack.push(12);
    // std::cout << stack.top() << "\n";
    // int& i = const_cast<int&>(stack.top());
    // i = 16;
    // std::cout << stack.top() << "\n";

    return 0;
}
