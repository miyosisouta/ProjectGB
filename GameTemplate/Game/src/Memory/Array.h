/**
 * AGEAHARJHREAJHREAJH
 * あｈｇｊらｈじょｒ；あｈｐｒ
 */
#pragma once
#include <iostream>
#include <memory>
#include <stdexcept>
#include <algorithm>


template <typename T>
class AllocatedArray
{
private:
    std::unique_ptr<T[]> data_;
    std::size_t size_; //!< アニメーションリストの配列のサイズを決めるため

public:
    // 型エイリアス（std::vector等との互換性のため）
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

    // コンストラクタ
    AllocatedArray()
        : data_(nullptr)
        , size_(0)
    {
    }

    // 初期サイズ指定のコンストラクタ
    explicit AllocatedArray(std::size_t size) : size_(0) {
        Create(size);
    }

    // コピーは禁止（固定配列の意図しないディープコピーを防ぐため）
    AllocatedArray(const AllocatedArray&) = delete;
    AllocatedArray& operator=(const AllocatedArray&) = delete;

    // ムーブは許可（関数の戻り値などに使えるように）
    AllocatedArray(AllocatedArray&& other) noexcept
        : data_(std::move(other.data_)), size_(other.size_) {
        other.size_ = 0;
    }
    AllocatedArray& operator=(AllocatedArray&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            size_ = other.size_;
            other.size_ = 0;
        }
        return *this;
    }

    // 実行時に配列の要素数を指定してメモリを確保する
    void Create(std::size_t size) {
        if (size > 0) {
            // C++14の std::make_unique で安全に配列を確保
            data_ = std::make_unique<T[]>(size);
            size_ = size;
        }
        else {
            data_.reset();
            size_ = 0;
        }
    }

    // --- std::vector のようなアクセス機能 ---

    // イテレータ
    iterator begin() { return data_.get(); }
    iterator end() { return data_.get() + size_; }
    const_iterator begin() const { return data_.get(); }
    const_iterator end() const { return data_.get() + size_; }
    const_iterator cbegin() const { return data_.get(); }
    const_iterator cend() const { return data_.get() + size_; }

    // 容量
    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // 要素アクセス
    reference operator[](size_type pos) { return data_[pos]; }
    const_reference operator[](size_type pos) const { return data_[pos]; }

    // 境界チェック付きアクセス
    reference at(size_type pos) {
        if (pos >= size_) throw std::out_of_range("FixedArray::at: index out of bounds");
        return data_[pos];
    }
    const_reference at(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("FixedArray::at: index out of bounds");
        return data_[pos];
    }

    // 生ポインタの取得
    pointer data() { return data_.get(); }
    const_pointer data() const { return data_.get(); }
};
