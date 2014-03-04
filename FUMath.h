/** License
 * vmath, set of classes for computer graphics mathematics.
 * Copyright (c) 2005-2011, Jan Bartipan < barzto at gmail dot com >
 * All rights reserved.
 * vmath is the base class for FUMath. FUMath basically adds a few more functions and changes the function and class names.
 */

namespace FUMath {
template<class T>
class FUVector2
{
public:
    union {
        //First element of vector, alias for X-coordinate.
        T x;

        //First element of vector, alias for S-coordinate. For textures notation.
        T s;
    };

    union {
        //Second element of vector, alias for Y-coordinate.
        T y;

        //Second element of vector, alias for T-coordinate.For textures notation.
        T t;
    };

    //----------------[ Constructors ]----------------//

    //Creates and sets to (0,0)
    FUVector2()
        : x(0), y(0)
    {
    }
    /**
     * @brief Creates and sets to (x,y)
     * @param nx initial x-coordinate value
     * @param ny initial y-coordinate value
     */
    FUVector2(T nx, T ny)
        : x(nx), y(ny)
    {
    }
    /**
     * @brief Copy constructor.
     * @param src --> Source of data for new created instance.
     */
    FUVector2(const FUVector2<T>& src)
        : x(src.x), y(src.y)
    {
    }
    /**
     * @brief Copy casting constructor.
     * @param src Source of data for new created instance.
     */
    template<class FromT>
    FUVector2(const FUVector2<FromT>& src)
        : x(static_cast<T>(src.x)), y(static_cast<T>(src.y))
    {
    }

    //----------------[ Access Operators ]----------------//
    /**
     * @brief Copy casting operator
     * @param rhs Right hand side argument of binary operator.
     */
    template<class FromT>
    FUVector2<T>& operator=(const FUVector2<FromT>& rhs) {
        x = static_cast<T>(rhs.x);
        y = static_cast<T>(rhs.y);
        return *this;
    }
    /**
     * @brief operator = --> Copy operator
     * @param rhs --> Right hand side argument of binary operator.
     * @return
     */
    FUVector2<T>& operator=(const FUVector2<T>& rhs) {
        x = rhs.x;
        y = rhs.y;
        return *this;
    }
    /**
     * @brief operator [] --> Array access operator
     * @param n --> Array index
     * @return For n = 0, reference to x coordinate, else reference to y y coordinate.
     */
    T& operator[](int n) {
        assert(n >= 0 && n <= 1);
        if (0 == n)
            return x;
        else
            return y;
    }

    /**
     * @brief Constant array access operator
     * @param n Array index
     * @return For n = 0, reference to x coordinate, else reference to y
     * y coordinate.
     */
    const T& operator[](int n) const {
        assert(n >= 0 && n <= 1);
        if (0 == n)
            return x;
        else
            return y;
    }

    //---------------[ Vector Aritmetic Operator ]---------------//
    /**
     * @brief Addition operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator+(const FUVector2<T>& rhs) const {
        return FUVector2<T>(x + rhs.x, y + rhs.y);
    }

    /**
     * @brief Subtraction operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator-(const FUVector2<T>& rhs) const {
        return FUVector2<T>(x - rhs.x, y - rhs.y);
    }

    /**
     * @brief Multiplication operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator*(const FUVector2<T>& rhs) const {
        return FUVector2<T>(x * rhs.x, y * rhs.y);
    }

    /**
     * @brief Division operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator/(const FUVector2<T>& rhs) const {
        return FUVector2<T>(x / rhs.x, y / rhs.y);
    }

    /**
     * @brief Addition operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator+=(const FUVector2<T>& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    /**
     * @brief Substraction operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator-=(const FUVector2<T>& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    /**
     * @brief Multiplication operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator*=(const FUVector2<T>& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    /**
     * @brief Division operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator/=(const FUVector2<T>& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    //--------------[ Scalar Vector Operator ]--------------//
    /**
     * @brief Addition operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator+(T rhs) const {
        return FUVector2<T>(x + rhs, y + rhs);
    }

    /**
     * @brief Subtraction operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator-(T rhs) const {
        return FUVector2<T>(x - rhs, y - rhs);
    }

    /**
     * @brief Multiplication operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator*(T rhs) const {
        return FUVector2<T>(x * rhs, y * rhs);
    }

    /**
     * @brief Division operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T> operator/(T rhs) const {
        return FUVector2<T>(x / rhs, y / rhs);
    }

    /**
     * @brief Addition operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator+=(T rhs) {
        x += rhs;
        y += rhs;
        return *this;
    }

    /**
     * @brief Subtraction operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator-=(T rhs) {
        x -= rhs;
        y -= rhs;
        return *this;
    }

    /**
     * @brief Multiplication operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator*=(T rhs) {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    /**
     * @brief Division operator
     * @param rhs Right hand side argument of binary operator.
     */
    FUVector2<T>& operator/=(T rhs) {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    //--------------[ Equality Operator ]--------------//
    /**
     * @brief Equality test operator
     * @param rhs Right hand side argument of binary operator.
     * @note Test of equality is based of threshold EPSILON value. To be two
     * values equal, must satisfy this condition | lhs.x - rhs.y | < EPSILON,
     * same for y-coordinate.
     */
    bool operator==(const FUVector2<T>& rhs) const {
        return (std::abs(x - rhs.x) < EPSILON) && (std::abs(y - rhs.y) < EPSILON);
    }

    /**
     * @brief Inequality test operator
     * @param rhs Right hand side argument of binary operator.
     * @return not (lhs == rhs) :-P
     */
    bool operator!=(const FUVector2<T>& rhs) const {
        return !(*this == rhs);
    }

    //-------------[ Unary Operations ]-------------//
    /**
     * @brief Unary negate operator
     * @return negated vector
     */
    FUVector2<T> operator-() const {
        return FUVector2<T>(-x, -y);
    }

    //-------------[ Size Operations ]-------------//
    /**
     * @brief Get length of vector.
     * @return lenght of vector
     */
    T length() const {
        return (T) std::sqrt(x * x + y * y);
    }
    /**
     * @brief Normalize vector
     */
    void normalize() {
        T s = length();
        x /= s;
        y /= s;
    }

    /**
     * @brief Returns x alias for the vector
     * @return
     */
    T getX() {
        return x;
    }

    /**
     * @brief Returns y alias for the vector
     * @return T --> y alias for the vector
     */
    T getY() {
        return y;
    }

    /**
     * @brief Sets x alias for the vector
     * @return
     */
    void setX(T X) {
        x = X;
    }

    /**
     * @brief Sets y alias for the vector
     * @return T --> y alias for the vector
     */
    void setY(T Y) {
        y = Y;
    }

    /**
     * @brief Return square of length.
     * @return length ^ 2
     * @note This method is faster then length(). For comparison of length of two vector can be used just this value, instead
     * of more expensive length() method.
     */
    T lengthSquare() const {
        return x * x + y * y;
    }

    //--------------[ Misc. Operations ]--------------//
    /**
     * @brief Linear interpolation of two vectors
     * @param fact --> Factor of interpolation. For translation from position
     * of this vector to vector r, values of factor goes from 0.0 to 1.0.
     * @param r --> Second Vector for interpolation
     * @note However values of fact parameter are reasonable only in interval
     * [0.0 , 1.0], you can pass also values outside of this interval and you can get result (extrapolation?)
     */
    FUVector2<T> linearInterpolation(T fact, const FUVector2<T>& r) const {
        return (*this) + (r - (*this)) * fact;
    }

    //-------------[ Conversion ]-------------//
    /**
     * @brief operator T * --> Conversion to pointer operator
     * @return Pointer to internally stored (in management of class FUVector2<T>) used for passing FUVector2<T> values to gl*2[fd] functions.
     */
    operator T*() {
        return (T*) this;
    }
    /**
     * @brief operator const T * --> Conversion to pointer operator
     * @return Constant Pointer to internally stored (in management of class FUVector2<T>)
     * used for passing FUVector2<T> values to gl*2[fd] functions.
     */
    operator const T*() const {
        return (const T*) this;
    }

    //-------------[ Output Operator ]-------------//
    /**
     * @brief operator << --> Output to stream operator
     * @param lhs --> Left hand side argument of operator (commonly ostream instance).
     * @param rhs --> Right hand side argument of operator.
     * @return Left hand side argument - the ostream object passed to operator.
     */
    friend std::ostream& operator<<(std::ostream& lhs, const FUVector2<T>& rhs) {
        lhs << "[" << rhs.x << "," << rhs.y << "]";
        return lhs;
    }
    /**
     * @brief Gets string representation.
     * @return returns std::string
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }
};
}
