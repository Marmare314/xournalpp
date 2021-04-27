#include "XmlPointNode.h"

#include "Util.h"

XmlPointNode::XmlPointNode(const char* tag): XmlAudioNode(tag) {}

void XmlPointNode::addPoint(const Point* point) { points.emplace_back(*point); }

void XmlPointNode::writeOut(OutputStream* out) {
    /** Write stroke and its attributes */
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    for (auto pointIter = points.begin(); pointIter != points.end(); pointIter++) {
        if (pointIter != points.begin()) {
            out->write(" ");
        }

        Util::writeCoordinateString(out, pointIter->x, pointIter->y);
    }

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
