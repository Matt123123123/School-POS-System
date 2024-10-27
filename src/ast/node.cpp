#include <ast/node.hpp>
#include <cassert>
#include <cmath>
#include <memory>
#include <optional>
#include <screen.hpp>
#include <string>
#include <utils.hpp>

using namespace std;
using namespace terminal;
using namespace string_utils;

Node::Node() : posX(0), posY(0), width(0), height(0){};

Node::Node(unsigned int col) : posX(col), posY(0), width(0), height(0){};

Node::Node(unsigned int col, unsigned int row)
    : posX(col), posY(row), width(0), height(0) {}

Node::~Node() = default;

NodeRenderStyle Node::nodeRenderStyle() const noexcept {
    return NodeRenderStyle::BLOCK;
}

bool Node::canHaveChildren() const noexcept { return true; }

unsigned int Node::getWidth() const noexcept { return width; }

void Node::setWidth(unsigned int w) { width = w; }

unsigned int Node::getHeight() const noexcept { return height; }

void Node::setHeight(unsigned int h) { height = h; }

unsigned int Node::getPosX() const noexcept { return posX; }

void Node::setPosX(unsigned int col) { posX = col; }

unsigned int Node::getPosY() const noexcept { return posY; }

void Node::setPosY(unsigned int row) { posY = row; }

Node::NodePtr Node::getParent() const noexcept { return parent; }

const vector<Node::NodePtr>& Node::getChildren() const noexcept {
    return children;
}

void Node::assertChildIsValid(NodePtr) const {}

void Node::setAsParentOfChild(NodePtr child) {
    child->parent = shared_from_this();
}

void Node::onChildRemoved(size_t idx, NodePtr child) {
    if (children.empty()) {
        setWidth(0);
        setHeight(0);

        return;
    }

    unsigned int heightOfRemovedChild = child->getHeight();
    unsigned int largestWidth = 0;

    for (NodePtr node : children) {
        unsigned nodeRealWidth = node->getPosX() + node->getWidth();

        if (largestWidth < nodeRealWidth) {
            largestWidth = nodeRealWidth;
        }
    }

    for (size_t i = idx, l = children.size(); i < l; ++i) {
        NodePtr node = children.at(i);

        node->setPosX(node->getPosX() - heightOfRemovedChild);
    }

    setWidth(largestWidth);
    setHeight(getHeight() - heightOfRemovedChild);
}

void Node::onChildAppended() {
    assert(!children.empty() ||
           !"Node::onChildAppended() called when children is empty().");

    unsigned int posX = getPosX();
    unsigned int posY = getPosY();
    size_t childrenSize = children.size();
    NodePtr appendedChild = children.at(childrenSize - 1);

    if (childrenSize > 1) {
        NodePtr prevSibling = children.at(childrenSize - 2);

        switch (appendedChild->nodeRenderStyle()) {
            case NodeRenderStyle::BLOCK: {
                appendedChild->setPosY(prevSibling->getPosY() +
                                       prevSibling->getHeight());
            }; break;
            case NodeRenderStyle::INLINE: {
                appendedChild->setPosX(prevSibling->getPosX() +
                                       prevSibling->getWidth());
            }; break;
        }
    } else {
        appendedChild->setPosX(posX);
        appendedChild->setPosY(posY);
    }

    unsigned int appendedChildRealWidth =
        appendedChild->getPosX() + appendedChild->getWidth();

    // TODO: Have this wrap to next line if this exceeds
    // screen size.
    if (getWidth() < appendedChildRealWidth) {
        setWidth(appendedChildRealWidth);
    }

    if (appendedChild->nodeRenderStyle() == NodeRenderStyle::BLOCK) {
        setHeight(getHeight() + appendedChild->getHeight());
    }
}

void Node::appendChild(NodePtr child) {
    assert(canHaveChildren() || !"Node cannot have children");
    assertChildIsValid(child);

    setAsParentOfChild(child);

    children.push_back(move(child));

    onChildAppended();
}

void Node::removeChildAt(size_t idx) {
    assert(!children.empty() ||
           !"Called Node::removeChildAt() with no children.");
    assert(
        idx < children.size() ||
        !"Passed in a size_t idx > children.size() at Node::removeChildAt().");

    NodePtr removedChild = children.at(idx);

    children.erase(children.begin() + idx);

    onChildRemoved(idx, removedChild);
}

void Node::removeAllChildren() noexcept {
    assert(canHaveChildren() ||
           !"Called Node::removeAllChildren() on a node that cannot have "
           "children.");

    children.clear();

    setWidth(0);
    setHeight(0);
}

void ContainerNode::render(ostringstream* buf) const {
    for (NodePtr node : children) {
        node->render(buf);
    }
}

NodeTypes ContainerNode::nodeType() const noexcept {
    return NodeTypes::CONTAINER;
}

GridNode::GridNode() : colGap(4), rowGap(2), childWidth(0) {
    setWidth(getScreen().getWidth());
}

GridNode::GridNode(unsigned int width) : colGap(4), rowGap(2), childWidth(0) {
    setWidth(width);
}

GridNode::GridNode(unsigned int width, unsigned int childWidth)
    : colGap(4), rowGap(2), childWidth(childWidth) {
    assert(childWidth > getWidth() ||
           !"child width should be <= grid container's width.");
    setWidth(width);
}

GridNode::GridNode(unsigned int width, unsigned int childWidth,
                   unsigned int colGap)
    : colGap(colGap), rowGap(2), childWidth(childWidth) {
    assert(childWidth > getWidth() ||
           !"child width should be <= grid container's width.");
    setWidth(width);
}

GridNode::GridNode(unsigned int width, unsigned int childWidth,
                   unsigned int colGap, unsigned int rowGap)
    : colGap(colGap), rowGap(rowGap), childWidth(childWidth) {
    assert(childWidth > getWidth() ||
           !"child width should be <= grid container's width.");
    setWidth(width);
}

void GridNode::removeAllChildren() noexcept {
    children.clear();

    setHeight(0);
}

unsigned int GridNode::getColGap() const noexcept { return colGap; }

void GridNode::setColGap(unsigned int c) {
    assert(c > 0 || !"column gap must be > 0");
    colGap = c;
}

unsigned int GridNode::getRowGap() const noexcept { return rowGap; }

void GridNode::setRowGap(unsigned int r) {
    assert(r > 0 || !"row gap must be > 0");
    rowGap = r;
}

void GridNode::onChildAppended() {
    assert(!children.empty() ||
           !"GridNode::onChildAppended() called when children is empty().");

    size_t childrenSize = children.size();

    NodePtr appendedChild = children.at(childrenSize - 1);

    if (childWidth != 0) {
        appendedChild->setWidth(childWidth);
    }

    if (childrenSize == 1) {
        appendedChild->setPosX(getPosX());
        appendedChild->setPosY(getPosY());

        if (childWidth == 0) {
            shared_ptr<GridNode> prnt =
                dynamic_pointer_cast<GridNode>(getParent());

            if (prnt) {
                appendedChild->setWidth(getWidth());
            }
        }

        setHeight(appendedChild->getHeight());

        return;
    }

    NodePtr prevSiblingOfAppendedChild = children.at(childrenSize - 2);
    unsigned int prevSiblingRealWidth = prevSiblingOfAppendedChild->getPosX() +
                                        prevSiblingOfAppendedChild->getWidth() +
                                        colGap;

    // have the appended child take the remaining width
    if (childWidth == 0 && prevSiblingRealWidth < getWidth()) {
        appendedChild->setWidth(getWidth() - prevSiblingRealWidth);
    }

    // if overflow
    if (prevSiblingRealWidth + appendedChild->getWidth() > getWidth()) {
        appendedChild->setPosX(getPosX());
        appendedChild->setPosY(prevSiblingOfAppendedChild->getPosY() +
                               prevSiblingOfAppendedChild->getHeight() +
                               rowGap);

        setHeight(appendedChild->getPosY() + appendedChild->getHeight());
    } else {
        unsigned int prevSiblingHeight =
            prevSiblingOfAppendedChild->getPosY() +
            prevSiblingOfAppendedChild->getHeight();
        unsigned int appendedChildHeight =
            appendedChild->getPosY() + appendedChild->getHeight();
        unsigned int tallestHeight = prevSiblingHeight > appendedChildHeight
                                         ? prevSiblingHeight
                                         : appendedChildHeight;

        for (size_t i = 0, l = childrenSize - 2; i < l; ++i) {
            NodePtr child = children.at(i);
            unsigned int childHeight = child->getPosY() + child->getHeight();

            if (childHeight > tallestHeight) {
                tallestHeight = childHeight;
            }
        }

        setHeight(tallestHeight);

        appendedChild->setPosX(prevSiblingRealWidth);
        appendedChild->setPosY(prevSiblingOfAppendedChild->getPosY());
    }
}

void GridNode::onChildRemoved(size_t idx, NodePtr removedChild) {
    throw logic_error("unimplemented");
}

bool LeafNode::canHaveChildren() const noexcept { return false; }

NodeTypes LeafNode::nodeType() const noexcept { return NodeTypes::LEAF; }

TextNode::TextNode(string text) : text(text) {
    setPosX(0);
    setPosY(0);
    setWidth(static_cast<unsigned int>(text.size()));
    setHeight(1);
}

TextNode::TextNode(string text, unsigned int posX) : text(text) {
    setPosX(posX);
    setPosY(0);
    setWidth(static_cast<unsigned int>(text.size()));
    setHeight(1);
}

TextNode::TextNode(string text, unsigned int posX, unsigned int posY)
    : text(text) {
    setPosX(posX);
    setPosY(posY);
    setWidth(static_cast<unsigned int>(text.size()));
    setHeight(1);
}

TextNode::TextNode(string text, unsigned int posX, unsigned int posY,
                   unsigned int width)
    : text(text) {
    setPosX(posX);
    setPosY(posY);
    setWidth(width);
    setHeight(1);
}

TextNode::TextNode(string text, unsigned int posX, unsigned int posY,
                   unsigned int width, unsigned int height)
    : text(text) {
    setPosX(posX);
    setPosY(posY);
    setWidth(width);
    setHeight(height);
}

NodeRenderStyle TextNode::nodeRenderStyle() const noexcept {
    return NodeRenderStyle::INLINE;
}

void TextNode::render(ostringstream* buf) const {
    moveCursorTo(buf, getPosX(), getPosY());

    if (color != nullopt) {
        tuple<uint8_t, uint8_t, uint8_t> clr = color.value();

        textForeground(buf, get<0>(clr), get<1>(clr), get<2>(clr));
    }

    if (backgroundColor != nullopt) {
        tuple<uint8_t, uint8_t, uint8_t> bgClr = backgroundColor.value();

        textBackground(buf, get<0>(bgClr), get<1>(bgClr), get<2>(bgClr));
    }

    for (const auto& format : formats) {
        switch (format) {
            case TextNodeFormats::BOLD:
                textBold(buf);
                break;
            case TextNodeFormats::STRIKETHROUGH:
                textStrikethrough(buf);
                break;
            case TextNodeFormats::DIM:
                textDim(buf);
                break;
            case TextNodeFormats::ITALIC:
                textItalic(buf);
                break;
            case TextNodeFormats::UNDERLINE:
                textUnderline(buf);
                break;
        }
    }

    size_t len = text.size();
    unsigned int currWidth = getWidth();
    unsigned int currHeight = getHeight();

    if (len > currWidth) {
        unsigned int currLine = 0;

        while (currLine <= currHeight) {
            unsigned int start = currLine * currWidth;

            if (start >= len) {
                break;
            }

            unsigned int end =
                start == 0 ? currWidth : static_cast<unsigned int>(len) - start;

            *buf << text.substr(static_cast<size_t>(start),
                                static_cast<size_t>(end));
            moveCursorTo(buf, getPosX(), getPosY() + (++currLine));
        }
    } else {
        *buf << text;

        if (len < currWidth) {
            *buf << string(currWidth - len, ' ');

            unsigned int currLine = 1;

            while (currLine < currHeight) {
                *buf << endl << string(len, ' ');
                ++currLine;
            }
        }
    }
}

void TextNode::setWidth(unsigned int w) {
    assert(w > 0 || !"TextNode::setWidth() received a width that's not > 0");

    unsigned int textSize = static_cast<unsigned int>(text.size());

    if (w < textSize) {
        height = static_cast<unsigned int>(
            ceil(static_cast<float>(textSize) / static_cast<float>(w)));

        if (height == 0) {
            height = 1;
        }
    }

    width = w;
}

void TextNode::setHeight(unsigned int h) {
    assert(h > 0 || !"TextNode::setHeight() received a height that's not > 0");

    size_t textSize = text.size();

    if (width < textSize) {
        assert(h < static_cast<unsigned int>(ceil(static_cast<float>(textSize) / static_cast<float>(width))) || !"TextNode::setHeight() received insufficient height to accommodate the wrapping of its text contents.");
    }

    height = h;
}

void TextNode::setRedColor(uint8_t r) {
    if (color == nullopt) {
        color = {r, 0, 0};
    } else {
        get<0>(color.value()) = r;
    }
}

void TextNode::setGreenColor(uint8_t g) {
    if (color == nullopt) {
        color = {0, g, 0};
    } else {
        get<1>(color.value()) = g;
    }
}

void TextNode::setBlueColor(uint8_t b) {
    if (color == nullopt) {
        color = {0, 0, b};
    } else {
        get<2>(color.value()) = b;
    }
}

void TextNode::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if (color == nullopt) {
        color = {r, g, b};
    } else {
        get<0>(color.value()) = r;
        get<1>(color.value()) = g;
        get<2>(color.value()) = b;
    }
}

void TextNode::resetColor() noexcept { color.reset(); }

void TextNode::setRedBackgroundColor(uint8_t r) {
    if (backgroundColor == nullopt) {
        backgroundColor = {r, 0, 0};
    } else {
        get<0>(backgroundColor.value()) = r;
    }
}

void TextNode::setGreenBackgroundColor(uint8_t g) {
    if (backgroundColor == nullopt) {
        backgroundColor = {0, g, 0};
    } else {
        get<1>(backgroundColor.value()) = g;
    }
}

void TextNode::setBlueBackgroundColor(uint8_t b) {
    if (backgroundColor == nullopt) {
        backgroundColor = {0, 0, b};
    } else {
        get<2>(backgroundColor.value()) = b;
    }
}

void TextNode::setBackgroundColor(uint8_t r, uint8_t g, uint8_t b) {
    if (backgroundColor == nullopt) {
        backgroundColor = {r, g, b};
    } else {
        get<0>(backgroundColor.value()) = r;
        get<1>(backgroundColor.value()) = g;
        get<2>(backgroundColor.value()) = b;
    }
}

void TextNode::resetBackgroundColor() noexcept { backgroundColor.reset(); }

void TextNode::setBold() { formats.insert(TextNodeFormats::BOLD); }

void TextNode::removeBold() { formats.erase(TextNodeFormats::BOLD); }

void TextNode::setItalic() { formats.insert(TextNodeFormats::ITALIC); }

void TextNode::removeItalic() { formats.erase(TextNodeFormats::ITALIC); }

void TextNode::setStrikethrough() {
    formats.insert(TextNodeFormats::STRIKETHROUGH);
}

void TextNode::removeStrikethrough() {
    formats.erase(TextNodeFormats::STRIKETHROUGH);
}

void TextNode::setDim() { formats.insert(TextNodeFormats::DIM); }

void TextNode::removeDim() { formats.erase(TextNodeFormats::DIM); }

void TextNode::setUnderline() { formats.insert(TextNodeFormats::UNDERLINE); }

void TextNode::removeUnderline() { formats.erase(TextNodeFormats::UNDERLINE); }

void TextNode::resetFormatting() { formats.clear(); }

LineBreakNode::LineBreakNode() { setHeight(1); }

LineBreakNode::LineBreakNode(unsigned int height) {
    assert(height > 0 || !"LineBreakNode() height must be > 0");
    setHeight(height);
}

LineBreakNode::LineBreakNode(unsigned int height, unsigned int posX) {
    assert(height > 0 || !"LineBreakNode() height must be > 0");
    setHeight(height);
    setPosX(posX);
}

LineBreakNode::LineBreakNode(unsigned int height, unsigned int posX,
                             unsigned int posY) {
    assert(height > 0 || !"LineBreakNode() height must be > 0");
    setHeight(height);
    setPosX(posX);
    setPosY(posY);
}

void LineBreakNode::render(ostringstream* buf) const {
    for (unsigned int i = 0; i < height; ++i) {
        moveCursorTo(buf, getPosX(), getPosY() + i);
        *buf << endl;
    }
}

SelectOptionNode::SelectOptionNode(string value)
    : TextNode(kebabToPascal(value)), value(value) {}

SelectOptionNode::SelectOptionNode(string value, unsigned int posX)
    : TextNode(kebabToPascal(value), posX), value(value) {}

SelectOptionNode::SelectOptionNode(string value, unsigned int posX,
                                   unsigned int posY)
    : TextNode(kebabToPascal(value), posX, posY), value(value) {}

SelectOptionNode::SelectOptionNode(string value, unsigned int posX,
                                   unsigned int posY, unsigned int width)
    : TextNode(kebabToPascal(value), posX, posY, width), value(value) {}

SelectOptionNode::SelectOptionNode(string value, unsigned int posX,
                                   unsigned int posY, unsigned int width,
                                   unsigned int height)
    : TextNode(kebabToPascal(value), posX, posY, width, height), value(value) {}

NodeRenderStyle SelectOptionNode::nodeRenderStyle() const noexcept {
    return NodeRenderStyle::BLOCK;
}

string SelectOptionNode::getValue() const noexcept { return value; }

void InteractableNode::render(ostringstream* buf) const {
    for (NodePtr node : children) {
        node->render(buf);
    }
}

bool InteractableNode::onKeyPressed(unsigned int keyCode) { return false; }

NodeTypes InteractableNode::nodeType() const noexcept {
    return NodeTypes::INTERACTABLE;
}

SelectNode::SelectNode() : activeOptionIdx(0) {}

void SelectNode::render(ostringstream* buf) const {
    for (size_t i = 0, l = children.size(); i < l; ++i) {
        shared_ptr<SelectOptionNode> node =
            static_pointer_cast<SelectOptionNode>(children.at(i));

        if (i == activeOptionIdx) {
            node->setBackgroundColor(255, 255, 255);
            node->setColor(0, 0, 0);
        } else {
            node->resetBackgroundColor();
            node->resetColor();
        }

        node->render(buf);
    }
}

void SelectNode::onChildRemoved(size_t idx, NodePtr removedChild) {
    if (children.empty()) {
        setWidth(0);
        setHeight(0);

        activeOptionIdx = 0;

        return;
    }

    unsigned int heightOfRemovedChild = removedChild->getHeight();
    unsigned int largestWidth = 0;

    for (NodePtr node : children) {
        unsigned nodeRealWidth = node->getPosX() + node->getWidth();

        if (largestWidth < nodeRealWidth) {
            largestWidth = nodeRealWidth;
        }
    }

    for (size_t i = idx, l = children.size(); i < l; ++i) {
        NodePtr node = children.at(i);

        node->setPosX(node->getPosX() - heightOfRemovedChild);
    }

    setWidth(largestWidth);
    setHeight(getHeight() - heightOfRemovedChild);

    // ==== Update option if active is removed  ==== //

    size_t childrenSize = children.size();

    if (activeOptionIdx == childrenSize) {
        activeOptionIdx = childrenSize - 1;
    } else if (idx < activeOptionIdx) {
        activeOptionIdx -= 1;
    }
}

void SelectNode::assertChildIsValid(NodePtr child) const {
    assert(dynamic_pointer_cast<SelectOptionNode>(child) ||
           !"SelectNode only accepts SelectOptionNode as its children");
}

optional<string> SelectNode::getValueOfSelectedOption() const {
    if (children.empty()) {
        return nullopt;
    }

    assert(activeOptionIdx < children.size() || !"SelectNode::getValueOfSelectedOption() is called when activeOptionIdx is < children.size()");

    shared_ptr<SelectOptionNode> selectedOptionNode =
        static_pointer_cast<SelectOptionNode>(children.at(activeOptionIdx));

    return selectedOptionNode->getValue();
}