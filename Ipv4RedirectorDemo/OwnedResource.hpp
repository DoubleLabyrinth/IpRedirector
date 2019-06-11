#pragma once
#include <type_traits>
#include <utility>

template<typename __ResourceTraits, typename __LambdaReleasor = void>
class OwnedResource {
    static_assert(std::is_pod<typename __ResourceTraits::HandleType>::value == true);
private:
    using HandleType = typename __ResourceTraits::HandleType;
    HandleType _Handle;
    __LambdaReleasor _Releasor;
public:

    OwnedResource(__ResourceTraits Traits, const __LambdaReleasor& LambdaRelasor) noexcept :
        _Handle(__ResourceTraits::DefaultInvalidValue),
        _Releasor(LambdaRelasor) {}

    OwnedResource(__ResourceTraits Traits, HandleType Handle, const __LambdaReleasor& LambdaRelasor) noexcept :
        _Handle(Handle),
        _Releasor(LambdaRelasor) {}

    OwnedResource(const OwnedResource<__ResourceTraits, __LambdaReleasor>& Other) = delete;

    OwnedResource(OwnedResource<__ResourceTraits, __LambdaReleasor>&& Other) noexcept :
        _Handle(Other._Handle),
        _Releasor(std::move(Other._Releasor)) 
    {
        Other._Handle = __ResourceTraits::DefaultInvalidValue;
    }

    OwnedResource<__ResourceTraits, __LambdaReleasor>& operator=(const OwnedResource<__ResourceTraits, __LambdaReleasor>& Other) = delete;

    OwnedResource<__ResourceTraits, __LambdaReleasor>& operator=(OwnedResource<__ResourceTraits, __LambdaReleasor>&& Other) noexcept {
        _Handle = Other._Handle;
        _Releasor = std::move(Other._Releasor);
        Other._Handle = __ResourceTraits::DefaultInvalidValue;
        return *this;
    }

    operator HandleType() const noexcept {
        return _Handle;
    }

    template<typename __AsType, typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    __AsType As() const noexcept {
        return reinterpret_cast<__AsType>(_Handle);
    }

    template<typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    HandleType operator->() noexcept {
        return _Handle;
    }

    template<typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    const HandleType operator->() const noexcept {
        return _Handle;
    }

    bool IsValid() const noexcept {
        return __ResourceTraits::IsValid(_Handle);
    }

    HandleType Get() const noexcept {
        return _Handle;
    }

    template<typename __ReturnType = HandleType*>
    __ReturnType GetAddress() noexcept {
        return &_Handle;
    }

    void TakeOver(HandleType Handle) {
        if (__ResourceTraits::IsValid(_Handle) == false) {
            _Handle = Handle;
        } else {
            _Releasor(_Handle);
            _Handle = Handle;
        }
    }

    void Abandon() noexcept {
        _Handle = __ResourceTraits::DefaultInvalidValue;
    }

    HandleType Transfer() noexcept {
        HandleType h = _Handle;
        _Handle = __ResourceTraits::DefaultInvalidValue;
        return h;
    }

    template<typename __AsType, typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    __AsType TransferAs() noexcept {
        __AsType h = reinterpret_cast<__AsType>(_Handle);
        _Handle = __ResourceTraits::DefaultInvalidValue;
        return h;
    }

    void Release() {
        if (__ResourceTraits::IsValid(_Handle)) {
            _Releasor(_Handle);
            _Handle = __ResourceTraits::DefaultInvalidValue;
        }
    }

    ~OwnedResource() {
        if (__ResourceTraits::IsValid(_Handle)) {
            _Releasor(_Handle);
            _Handle = __ResourceTraits::DefaultInvalidValue;
        }
    }
};

template<typename __ResourceTraits>
class OwnedResource<__ResourceTraits, void> {
    static_assert(std::is_pod<typename __ResourceTraits::HandleType>::value == true);
private:
    using HandleType = typename __ResourceTraits::HandleType;
    HandleType _Handle;
public:

    OwnedResource(__ResourceTraits Traits) noexcept :
        _Handle(__ResourceTraits::DefaultInvalidValue) {}

    OwnedResource(__ResourceTraits Traits, HandleType Handle) noexcept :
        _Handle(Handle) {}

    OwnedResource(const OwnedResource<__ResourceTraits, void>& Other) = delete;

    OwnedResource(OwnedResource<__ResourceTraits, void>&& Other) noexcept :
        _Handle(Other._Handle) 
    {
        Other._Handle = __ResourceTraits::DefaultInvalidValue;
    }

    OwnedResource<__ResourceTraits, void>& operator=(const OwnedResource<__ResourceTraits, void>& Other) = delete;

    OwnedResource<__ResourceTraits, void>& operator=(OwnedResource<__ResourceTraits, void>&& Other) noexcept {
        _Handle = Other._Handle;
        Other._Handle = __ResourceTraits::DefaultInvalidValue;
        return *this;
    }

    operator HandleType() const noexcept {
        return _Handle;
    }

    template<typename __AsType, typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    __AsType As() const noexcept {
        return reinterpret_cast<__AsType>(_Handle);
    }

    template<typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    HandleType operator->() noexcept {
        return _Handle;
    }

    template<typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    const HandleType operator->() const noexcept {
        return _Handle;
    }

    bool IsValid() const noexcept {
        return __ResourceTraits::IsValid(_Handle);
    }

    HandleType Get() const noexcept {
        return _Handle;
    }

    template<typename __ReturnType = HandleType * >
    __ReturnType GetAddress() noexcept {
        return &_Handle;
    }

    void TakeOver(HandleType Handle) {
        if (__ResourceTraits::IsValid(_Handle) == false) {
            _Handle = Handle;
        } else {
            __ResourceTraits::Releasor(_Handle);
            _Handle = Handle;
        }
    }

    void Abandon() noexcept {
        _Handle = __ResourceTraits::DefaultInvalidValue;
    }

    HandleType Transfer() noexcept {
        HandleType h = _Handle;
        _Handle = __ResourceTraits::DefaultInvalidValue;
        return h;
    }

    template<typename __AsType, typename __Dummy = typename std::enable_if<std::is_pointer<HandleType>::value>::type>
    __AsType TransferAs() noexcept {
        __AsType h = reinterpret_cast<__AsType>(_Handle);
        _Handle = __ResourceTraits::DefaultInvalidValue;
        return h;
    }

    void Release() {
        if (__ResourceTraits::IsValid(_Handle)) {
            __ResourceTraits::Releasor(_Handle);
            _Handle = __ResourceTraits::DefaultInvalidValue;
        }
    }

    ~OwnedResource() {
        if (__ResourceTraits::IsValid(_Handle)) {
            __ResourceTraits::Releasor(_Handle);
            _Handle = __ResourceTraits::DefaultInvalidValue;
        }
    }
};

template<typename __ResourceTraits, typename __LambdaReleasor>
OwnedResource(__ResourceTraits Traits, const __LambdaReleasor& LambdaRelasor) ->
    OwnedResource<__ResourceTraits, __LambdaReleasor>;

template<typename __ResourceTraits, typename __LambdaReleasor>
OwnedResource(__ResourceTraits Traits, typename __ResourceTraits::HandleType Handle, const __LambdaReleasor& LambdaRelasor) -> 
    OwnedResource<__ResourceTraits, __LambdaReleasor>;

template<typename __ResourceTraits>
OwnedResource(__ResourceTraits Traits) -> 
    OwnedResource<__ResourceTraits, void>;

template<typename __ResourceTraits>
OwnedResource(__ResourceTraits Traits, typename __ResourceTraits::HandleType Handle) -> 
    OwnedResource<__ResourceTraits, void>;

template<typename __ClassType>
struct CppObjectTraits {
    using HandleType = __ClassType*;

    static inline const HandleType DefaultInvalidValue = nullptr;

    static constexpr bool IsValid(const HandleType& Handle) noexcept {
        return Handle != DefaultInvalidValue;
    }

    static void Releasor(const HandleType& Handle) {
        delete Handle;
    }
};

template<typename __ClassType>
struct CppDynamicArrayTraits {
    using HandleType = __ClassType*;

    static inline const HandleType DefaultInvalidValue = nullptr;

    static constexpr bool IsValid(const HandleType& Handle) noexcept {
        return Handle != DefaultInvalidValue;
    }

    static void Releasor(const HandleType& Handle) {
        delete[] Handle;
    }
};
