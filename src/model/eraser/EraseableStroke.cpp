#include "EraseableStroke.h"

#include <cmath>

#include "model/Stroke.h"

#include "EraseableStrokePart.h"
#include "PartList.h"
#include "Range.h"

EraseableStroke::EraseableStroke(Stroke* stroke): stroke(stroke) {
    this->parts = new PartList();
    g_mutex_init(&this->partLock);

    for (int i = 1; i < stroke->getPointCount(); i++) {
        this->parts->add(new EraseableStrokePart(stroke->getPoint(i - 1), stroke->getPoint(i)));
    }
}

EraseableStroke::~EraseableStroke() {
    delete this->parts;
    this->parts = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// This is done in a Thread, every thing else in the main loop /////////////////
////////////////////////////////////////////////////////////////////////////////

void EraseableStroke::draw(cairo_t* cr) {
    g_mutex_lock(&this->partLock);
    PartList* tmpCopy = this->parts->clone();
    g_mutex_unlock(&this->partLock);

    double w = this->stroke->getWidth();

    for (GList* l = tmpCopy->data; l != nullptr; l = l->next) {
        auto* part = static_cast<EraseableStrokePart*>(l->data);
        if (part->getWidth() == Point::NO_PRESSURE) {
            cairo_set_line_width(cr, w);
        } else {
            cairo_set_line_width(cr, part->getWidth());
        }

        std::vector<Point>& pl = part->getPoints();
        cairo_move_to(cr, pl[0].x, pl[0].y);

        for (auto pointIter = pl.begin() + 1; pointIter != pl.end(); pointIter++) {
            cairo_line_to(cr, pointIter->x, pointIter->y);
        }
        cairo_stroke(cr);
    }

    delete tmpCopy;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The only public method
 */
auto EraseableStroke::erase(double x, double y, double halfEraserSize, Range* range) -> Range* {
    this->repaintRect = range;

    g_mutex_lock(&this->partLock);
    PartList* tmpCopy = this->parts->clone();
    g_mutex_unlock(&this->partLock);

    for (GList* l = tmpCopy->data; l != nullptr;) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
        l = l->next;
        erase(x, y, halfEraserSize, p, tmpCopy);
    }

    g_mutex_lock(&this->partLock);
    PartList* old = this->parts;
    this->parts = tmpCopy;
    g_mutex_unlock(&this->partLock);

    delete old;

    return this->repaintRect;
}

void EraseableStroke::addRepaintRect(double x, double y, double width, double height) {
    if (this->repaintRect) {
        this->repaintRect->addPoint(x, y);
    } else {
        this->repaintRect = new Range(x, y);
    }

    this->repaintRect->addPoint(x + width, y + height);
}

void EraseableStroke::erase(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list) {
    if (part->getPoints().size() < 2) {
        return;
    }

    Point eraser(x, y);

    Point a = part->getPoints().front();
    Point b = part->getPoints().back();

    if (eraser.lineLengthTo(a) < halfEraserSize * 1.2 && eraser.lineLengthTo(b) < halfEraserSize * 1.2) {
        list->data = g_list_remove(list->data, part);
        addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());

        delete part;
        return;
    }

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    double aX = a.x;
    double aY = a.y;
    double bX = b.x;
    double bY = b.y;

    // check first point
    if (aX >= x1 && aY >= y1 && aX <= x2 && aY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
            addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
            part->calcSize();
        }

        if (deleteAfter) {
            delete part;
        }

        return;
    }

    // check last point
    if (bX >= x1 && bY >= y1 && bX <= x2 && bY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
            addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
            part->calcSize();
        }

        if (deleteAfter) {
            delete part;
        }

        return;
    }

    double len = hypot(bX - aX, bY - aY);
    /**
     * The distance of the center of the eraser box to the line passing through (aX, aY) and (bX, bY)
     */
    double p = std::abs((x - aX) * (aY - bY) + (y - aY) * (bX - aX)) / len;

    // If the distance p of the center of the eraser box to the (full) line is in the range,
    // we check whether the eraser box is not too far from the line segment through the two points.

    if (p <= halfEraserSize) {
        double centerX = (aX + bX) / 2;
        double centerY = (aY + bY) / 2;
        double distance = hypot(x - centerX, y - centerY);

        // For the above check we imagine a circle whose center is the mid point of the two points of the stroke
        // and whose radius is half the length of the line segment plus half the diameter of the eraser box
        // plus some small padding
        // If the center of the eraser box lies within that circle then we consider it to be close enough

        distance -= halfEraserSize * std::sqrt(2);

        constexpr double PADDING = 0.1;

        if (distance <= len / 2 + PADDING) {
            bool deleteAfter = false;

            if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
                addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
                part->calcSize();
            }

            if (deleteAfter) {
                delete part;
            }

            return;
        }
    }
}

auto EraseableStroke::erasePart(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list,
                                bool* deleteStrokeAfter) -> bool {
    bool changed = false;

    part->splitFor(halfEraserSize);

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    /**
     * erase the beginning
     */

    std::vector<Point>& points = part->getPoints();

    for (auto pointIter = points.begin(); pointIter != points.end();) {
        if (pointIter->x >= x1 && pointIter->y >= y1 && pointIter->x <= x2 && pointIter->y <= y2) {
            pointIter = points.erase(pointIter);
            changed = true;
        } else {
            // only the beginning is handled here
            break;
        }
    }

    /**
     * erase the end
     */
    // ugly loop avoiding reverse_iterators
    for (auto pointIter = points.end() - 1; pointIter-- != points.begin();) {
        if (pointIter->x >= x1 && pointIter->y >= y1 && pointIter->x <= x2 && pointIter->y <= y2) {
            points.erase(pointIter);
            changed = true;
        } else {
            // only the end is handled here
            break;
        }
    }

    /**
     * handle the rest
     */

    std::vector<std::vector<Point>> splitPoints{{}};

    for (auto pointIter = points.begin(); pointIter != points.end();) {
        if (pointIter->x >= x1 && pointIter->y >= y1 && pointIter->x <= x2 && pointIter->y <= y2) {
            pointIter = points.erase(pointIter);
            if (!splitPoints.back().empty()) {
                splitPoints.push_back({});
            }
            changed = true;
        } else {
            splitPoints.back().push_back(*pointIter);
            pointIter++;
        }
    }
    if (splitPoints.back().empty()) {
        splitPoints.pop_back();
    }

    points.clear();
    if (!splitPoints.empty()) {
        points = std::move(splitPoints.front());
        splitPoints.erase(splitPoints.begin());

        int pos = g_list_index(list->data, part) + 1;

        // create data structure for all new (splitted) parts
        for (auto& l: splitPoints) {
            auto* newPart = new EraseableStrokePart(part->getWidth());
            newPart->getPoints() = std::move(l);
            list->data = g_list_insert(list->data, newPart, pos++);
        }
    } else {
        // no parts, all deleted
        list->data = g_list_remove(list->data, part);
        *deleteStrokeAfter = true;
    }

    return changed;
}

auto EraseableStroke::getStroke(Stroke* original) -> GList* {
    GList* list = nullptr;

    Stroke* s = nullptr;
    Point lastPoint(NAN, NAN);
    for (GList* l = this->parts->data; l != nullptr; l = l->next) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
        std::vector<Point>& points = p->getPoints();
        if (points.size() < 2) {
            continue;
        }

        Point a = points.front();
        Point b = points.back();
        a.z = p->getWidth();

        if (!lastPoint.equalsPos(a) || s == nullptr) {
            if (s) {
                s->addPoint(lastPoint);
            }
            s = new Stroke();
            s->setColor(original->getColor());
            s->setToolType(original->getToolType());
            s->setLineStyle(original->getLineStyle());
            s->setWidth(original->getWidth());
            list = g_list_append(list, s);
        }
        s->addPoint(a);
        lastPoint = b;
    }
    if (s) {
        s->addPoint(lastPoint);
    }

    return list;
}
