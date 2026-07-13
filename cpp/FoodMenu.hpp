#ifndef FOODMENU_HPP
#define FOODMENU_HPP

#include <string>
#include <vector>

struct FoodItem {
    std::string name;
    int price;
};

inline const std::vector<FoodItem>& foodMenu() {
    static const std::vector<FoodItem> menu = {
        {"SHAHI PANEER", 200}, {"KADAI PANEER", 220}, {"CHEESE KORMA", 160},
        {"MALAI KOFTA", 130}, {"MATAR PANEER", 125}, {"PALAK PANEER", 110},
        {"MIX VEGITABLES", 150}, {"RAJMA MAKHANI", 160}, {"DAL MAKHANI", 140},
        {"SALAD(GREEN)", 100}, {"DUM ALOO", 110}, {"MUSHROOM PANEER", 180},
        {"MASALA DOSA", 120}, {"MASALA CHICKEN", 80}, {"DAHI VADA", 100},
        {"MUTTON KORMA", 250}, {"MUTTON CURRY", 300}, {"EGG CURRY", 150},
        {"BUTTER CHICKEN", 240}, {"FISH CURRY", 200}, {"CHICKEN CURRY", 260},
        {"CHILLI CHICKEN", 250}, {"CHICKEN TIKKA", 150}, {"SEEKH KABAB", 230},
        {"CHICKEN TANDORI", 240}, {"PANEER TIKKA", 160}, {"CHICKEN SEEKHKABAB", 260},
        {"CHICKEN BIRYANI", 250}, {"PANEER PULAO", 220}, {"VEG.PULAO", 200},
        {"RUMALI ROTI", 40}, {"ALOO PARATHA", 30}, {"PANEER PARANTHA", 35},
        {"PIZZA MARGHERITA", 300}, {"VEG.BURGER", 200}, {"CHICKEN BURGER", 240},
        {"PANEER BURGER", 220}, {"CHEESE SANDWICH", 150}, {"TEA", 20},
        {"COFFEE", 20}, {"COLD COFFEE", 30}, {"LASSI", 30}, {"HOT CHOCOLATE", 50},
        {"SOFT DRINKS", 50}, {"COLD DRINKS", 60}, {"STRAWBERRY", 30},
        {"CHOCOLATE", 40}, {"BLACK FOREST", 60}, {"VANILLA", 30}, {"PISTA BADAM", 40}};
    return menu;
}

#endif
