#include <contrib.hpp>
#include <state.hpp>

using namespace std;

MenuItem::MenuItem(string id) : id(id) {}

string MenuItem::getId() const noexcept { return id; }

void MenuItem::setDescription(string desc) { description = desc; }

string MenuItem::getDescription() const noexcept { return description; }

uint8_t MenuItem::getQty() const noexcept { return qty; }

void MenuItem::increaseQty(uint8_t amount) {
   qty+=amount;
}

void MenuItem::decreaseQty(uint8_t amount) {
    if(qty==0)
        return;
    if(qty<amount)
        throw logic_error("qty<amount");
    
    qty-=amount;
}

void MenuItem::resetQty() noexcept { qty=0; }

double MenuItem::getPrice() const noexcept { return price; }

double MenuItem::calculateSubTotal() const noexcept {
   return qty*price;
}

double calculateChange(double givenCash, double price) {
    throw logic_error("unimplemented");
}

double calculateTotalOfChosenMenuItems(const vector<MenuItem>&) {
    throw logic_error("unimplemented");
}

void State::appendMenuItem(MenuItem menuItem) { menuItems.push_back(menuItem); }

void State::removeMenuItemWithId(string itemId) {
    throw logic_error("unimplemented");
}

optional<MenuItem*> State::getMenuItemWithId(string itemId) {
    throw logic_error("unimplemented");
}

const vector<MenuItem>& State::getMenuItems() const noexcept {
    return menuItems;
}

string formatNumber(double num) { throw logic_error("unimplemented"); }
