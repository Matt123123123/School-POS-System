#include <contrib/menu.hpp>

MenuItem::MenuItem(const string& id, const string& n, const double& bp,
                   const MenuItemSizes& s, const uint8_t& q,
                   const optional<string>& rm)
    : uid(id), name(n), basePrice(bp), size(s), qty(q), remarks(rm) {}

MenuItemSizeData::MenuItemSizeData(const MenuItemSizes& s, const string& desc,
                                   const double& additionalP)
    : size(s), description(desc), additionalPrice(additionalP) {}

MenuItemSizes MenuItemSizeData::getSize() const noexcept { return size; }


string MenuItemSizeData::getDescription() const noexcept {return description; }

double MenuItemSizeData::getAdditionalPrice() const noexcept {
    return additionalPrice;
}

string toString(const MenuItemSizes& size) noexcept {
    switch (size) {
        case MenuItemSizes::TALL:
            return "TALL";
        case MenuItemSizes::GRANDE:
            return "GRANDE";
        case MenuItemSizes::VENTI:
            return "VENTI";
        case MenuItemSizes::TRENTA:
            return "TRENTA";
    }

    return "";
}

MenuItemSizes fromString(const string& stringifiedSize) {
    if (stringifiedSize == "TALL") {
        return MenuItemSizes::TALL;
    }

    if (stringifiedSize == "GRANDE") {
        return MenuItemSizes::GRANDE;
    }

    if (stringifiedSize == "VENTI") {
        return MenuItemSizes::VENTI;
    }

    if (stringifiedSize == "TRENTA") {
        return MenuItemSizes::TRENTA;
    }

    assert(false);
}

double getAdditionalPriceForMenuItemSize(const MenuItemSizes& size) noexcept {
    switch (size) {
        case MenuItemSizes::TALL:
            return 0;
        case MenuItemSizes::GRANDE:
            return 10;
        case MenuItemSizes::VENTI:
            return 20;
        case MenuItemSizes::TRENTA:
            return 30;
    }

    return 0;
}

MenuItemAddonData::MenuItemAddonData(const string& n, const double& p)
    : name(n), price(p) {}
MenuItemAddonData::MenuItemAddonData(const string& n, const double& p,
                                     const string& desc)
    : name(n), price(p) {
    setDescription(desc);
}

string MenuItemAddonData::getName() const noexcept { return name; }

double MenuItemAddonData::getPrice() const noexcept { return price; }

string MenuItemAddonData::getDescription() const noexcept {
    return description;
}

void MenuItemAddonData::setDescription(const string& desc) {
    assert(desc.size() <= 250);

    description = desc;
}

MenuItemAddon::MenuItemAddon(const string& menuItemUid,
                             const MenuItemAddonData& data)
    : menuItemUid(menuItemUid),
      name(data.getName()),
      price(data.getPrice()),
      qty(1) {}

string MenuItemAddon::getName() const noexcept { return name; }

double MenuItemAddon::getPrice() const noexcept { return price; }

string MenuItemAddon::getMenuItemUid() const noexcept { return menuItemUid; }

MenuItemData::MenuItemData(const string& n, const double& bp)
    : name(kebabToPascal(n)), basePrice(bp) {}
MenuItemData::MenuItemData(const string& n, const double& bp,
                           const string& desc)
    : name(kebabToPascal(n)), basePrice(bp) {
    setDescription(desc);
}

string MenuItemData::getName() const noexcept { return name; }

string MenuItemData::getDescription() const noexcept { return description; }

void MenuItemData::setDescription(const string& desc) {
    assert(desc.size() <= 250);

    description = desc;
}

double MenuItemData::getBasePrice() const noexcept { return basePrice; }

MenuItem::MenuItem(const MenuItemData& data)
    : uid(genRandomID(8)),
      name(data.getName()),
      basePrice(data.getBasePrice()),
      size(MenuItemSizes::TALL),
      qty(1) {}

MenuItem::MenuItem(const MenuItemData& data, const MenuItemSizes& size)
    : uid(genRandomID(8)),
      name(data.getName()),
      basePrice(data.getBasePrice()),
      size(size),
      qty(1) {}

string MenuItem::getUid() const noexcept { return uid; }

string MenuItem::getName() const noexcept { return name; }

double MenuItem::getBasePrice() const noexcept { return basePrice; }

MenuItemSizes MenuItem::getSize() const noexcept { return size; }
void MenuItem::setSize(MenuItemSizes s) noexcept { size = s; }

uint8_t MenuItem::getQty() const noexcept { return qty; }

void MenuItem::increaseQty(const uint8_t& amount) { qty += amount; }

void MenuItem::decreaseQty(const uint8_t& amount) {
    if (qty == 0) {
        return;
    }

    assert(amount <= qty);

    qty -= amount;
}

void MenuItem::resetQty() noexcept { qty = 0; }

optional<string> MenuItem::getRemarks() const noexcept { return remarks; }

void MenuItem::setRemarks(const string& remark) {
    assert(remark.size() <= 50);

    remarks = remark;
}

double MenuItem::calculateSubtotal() const noexcept {
    return (basePrice + getAdditionalPriceForMenuItemSize(size)) * qty;
}
