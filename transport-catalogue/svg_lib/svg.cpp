#define _USE_MATH_DEFINES
#include <cmath>

#include "svg.h"
#include <map>


namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const StrokeLineCap stroke_line) {
    switch (stroke_line) {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        default:
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin stroke_line_join) {
    switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        default:
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Color color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}


void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << '\n';
}

// ---------- RenderContex ------------------

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// ---------- Document -----------------

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << '\n';
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << '\n';
    for (auto& obj : objects_) {
        obj->Render({out, 1, 2});
    }
    out << "</svg>"sv;
}

using namespace shapes;
// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

void Circle::Draw(ObjectContainer& container) const {
    container.Add(std::move(*this));
}

// ---------- Polyline -----------------
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {

    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    
    return polyline;
}

void Star::Draw(ObjectContainer& container) const {
    Polyline polyline = CreateStar(center_, outer_radius_, inner_radius_, num_rays_);
    polyline.SetFillColor("red");
    polyline.SetStrokeColor("black");
    container.Add(std::move(polyline));
}

void Polyline::RenderObject(const RenderContext& context) const  {
    auto& out = context.out;
    bool is_first = true;
    out << "<polyline points=\""sv;
    for (auto point : points_) {
        if (is_first) {
            is_first = false;
        } else {
            out << " "sv;
        }
        out << point.x << "," << point.y;
    }
    out << "\"";
    RenderAttrs(out);
    
    out << "/>"sv;
}

// ---------- Text -----------------

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos){
    position_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size){
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data){
    for (char& c : data){
        switch (c) {
            case '"':
                data_ += "&quot;";
                break;
                
            case '\'':
                data_ += "&apos;";
                break;
                
            case '<':
                data_ += "&lt;";
                break;
                
            case '>':
                data_ += "&gt;";
                break;
                
            case '&':
                data_ += "&amp;";
                break;
                
            default:
                data_.push_back(c);
                break;
        }
    }
    
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
//    static std::regex reg_quotes("\"");
//    static std::regex reg_ap("'");
//    static std::regex reg_l_br("<");
//    static std::regex reg_r_br(">");
//    static std::regex amp("&");
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
    out << data_;
    out << "</text>"sv;

}

// ---------- Triangle -----------------

void Triangle::Draw(ObjectContainer& container) const {
    container.Add(Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}
// ---------- Snowman -----------------

void Snowman::Draw(ObjectContainer& container) const {
    Circle bottom;
    bottom.SetCenter({head_center_.x, head_center_.y + (head_radius_ * 5)});
    bottom.SetRadius(head_radius_ * 2);
    bottom.SetFillColor("rgb(240,240,240)");
    bottom.SetStrokeColor("black");
    Circle middle;
    middle.SetCenter({head_center_.x, head_center_.y + (head_radius_ * 2)});
    middle.SetRadius(head_radius_ * 1.5);
    middle.SetFillColor("rgb(240,240,240)");
    middle.SetStrokeColor("black");
    Circle head;
    head.SetCenter(head_center_);
    head.SetRadius(head_radius_);
    head.SetFillColor("rgb(240,240,240)");
    head.SetStrokeColor("black");
    container.Add(std::move(bottom));
    container.Add(std::move(middle));
    container.Add(std::move(head));
}

}  // namespace svg
