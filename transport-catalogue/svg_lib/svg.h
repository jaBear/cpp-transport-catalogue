#pragma once

#define _USE_MATH_DEFINES

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Rgb {
    Rgb(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : red(red), green(green), blue(blue) {};
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, double opacity = 1.0) : red(red), green(green), blue(blue), opacity(opacity) {};
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;


// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor("none");



struct ColorPrinter {
    std::ostream& out;
    
    void operator()(std::monostate) const {
        out << "none";
    }
    void operator()(std::string Color) const {
        out << Color;
    }
    void operator()(Rgb Color) const {
        out << "rgb(" << static_cast<int>(Color.red) << "," << static_cast<int>(Color.green) << "," << static_cast<int>(Color.blue) << ")";
    }
    void operator()(Rgba Color) const {
        out << "rgba(" << static_cast<int>(Color.red) << "," << static_cast<int>(Color.green) << "," << static_cast<int>(Color.blue) << "," << Color.opacity << ")";
    }
};

//std::ostream& operator<<(std::ostream& out, const Color color);


enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineCap stroke_line);
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin stroke_line_join);
std::ostream& operator<<(std::ostream& out, const Color color);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */

struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const;

    void RenderIndent() const;

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    
    Owner& SetStrokeWidth(int width) {
        stroke_width_ = width;
        return AsOwner();
    }
    
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = std::move(line_cap);
        return AsOwner();
    }
    
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = std::move(line_join);
        return AsOwner();
    }
    
    

protected:
    ~PathProps() = default;

    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(ColorPrinter{out}, fill_color_.value());
            out << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{out}, stroke_color_.value());
            out << "\""sv;
        }
        
        if (stroke_width_) {
            if (std::holds_alternative<int>(*stroke_width_)) {
                out << " stroke-width=\""sv << get<int>(*stroke_width_) << "\""sv;
            } else {
                out << " stroke-width=\""sv << get<double>(*stroke_width_) << "\""sv;
            }
        }
        
        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }
        
        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<std::variant<int, double>> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */

// ---------- Object ------------------

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
public:
    
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::move(std::make_unique<Obj>(obj)));
    }
    virtual ~ObjectContainer() = default;

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
protected:
    explicit ObjectContainer() = default;
};

// ---------- Drawable ------------------

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;

protected:
    explicit Drawable() = default;
};
// ---------- Shapes ---------------------

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */

class Document : public ObjectContainer {
public:
    
    template <typename Obj>
    void Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }
    
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
        
private:
    
    std::vector<std::unique_ptr<Object>> objects_;

    // Прочие методы и данные, необходимые для реализации класса Document
};

namespace shapes {

class Circle final : public Object, public PathProps<Circle>, public Drawable {
public:
//    Circle(Point center, double radius) : center_(center), radius_(radius) {};
    
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
    
    void Draw(ObjectContainer& container) const override;


private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    
    std::vector<Point> points_;
    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
};


class Triangle final : public Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(ObjectContainer& container) const override;

private:
    Point p1_, p2_, p3_;
};

class Star final : public Drawable {
public:
    Star(Point point, double outer_radius, double inner_radius, int num_rays) : center_(point), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays){}
    
    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(ObjectContainer& container) const override;
    
private:
    Point center_;
    double outer_radius_;
    double inner_radius_;
    int num_rays_;
};
class Snowman final : public Drawable {
public:
    Snowman(Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius) {}
    
    void Draw(ObjectContainer& container) const override;
    
private:
    Point head_center_;
    double head_radius_;
};


/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text>{
public:
    void SymbolReplacement();
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;
    
    Point position_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_ = "";
};

} // namespace shapes


}  // namespace svg
