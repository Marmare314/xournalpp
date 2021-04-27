/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include "model/Point.h"

#include "XmlAudioNode.h"

class XmlPointNode: public XmlAudioNode {
public:
    XmlPointNode(const char* tag);

    XmlPointNode(const XmlPointNode& node) = delete;
    void operator=(const XmlPointNode& node) = delete;

public:
    void addPoint(const Point* point);
    void writeOut(OutputStream* out) override;

private:
    std::vector<Point> points{};
};
