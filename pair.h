#pragma once
template<typename T1, typename T2>
struct MyPair {
    T1 first;
    T2 second;
    MyPair() : first(T1()), second(T2()) {}
    MyPair(const T1& f, const T2& s) : first(f), second(s) {}
};
