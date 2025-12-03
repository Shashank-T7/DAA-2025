#include <bits/stdc++.h>
using namespace std;

struct Product {
    int id;
    string name;
    string vendor;
    double basePrice;
    int demand;          // recent demand score
    double currentPrice; // dynamic price
};

// For heap: highest demand first
struct DemandCmp {
    bool operator()(const Product &a, const Product &b) const {
        return a.demand < b.demand; // max-heap
    }
};

// ---------- GLOBAL STATE ----------
vector<Product> products;                           // array of products
unordered_map<int, double> priceCache;             // id -> current price (hashing)
unordered_map<int, string> vendorLookup;           // id -> vendor (lookup table)
unordered_map<string, vector<int>> vendorProducts; // vendor -> product ids

// ---------- DYNAMIC PRICING LOGIC ----------
double computeDynamicPrice(const Product &p, int timeOfDayHour,
                           double competitorPrice) {
    // Simple illustrative model:
    // base adjusted by demand and time of day vs competitor
    double price = p.basePrice;

    // demand factor: up to +20%
    double demandFactor = 1.0 + min(0.20, p.demand * 0.01);
    price *= demandFactor;

    // time-of-day factor: peak hours 19-22 → +10%, low hours 2-5 → -10%
    if (timeOfDayHour >= 19 && timeOfDayHour <= 22) {
        price *= 1.10;
    } else if (timeOfDayHour >= 2 && timeOfDayHour <= 5) {
        price *= 0.90;
    }

    // competitor adjustment: try to be 5% cheaper if competitor is lower
    if (competitorPrice > 0 && competitorPrice < price) {
        price = competitorPrice * 0.95;
    }

    return price;
}

void recomputeAllPrices(int hour) {
    cout << "Enter competitor price for each product (enter -1 if unknown):\n";
    for (auto &p : products) {
        double competitor;
        cout << "Product " << p.id << " (" << p.name << "): ";
        cin >> competitor;
        if (competitor < 0) competitor = 0;

        p.currentPrice = computeDynamicPrice(p, hour, competitor);
        priceCache[p.id] = p.currentPrice;
    }
    cout << "Dynamic prices recomputed.\n";
}

// ---------- PRODUCT OPS ----------
void addProduct() {
    Product p;
    cout << "Enter product id: ";
    cin >> p.id;
    cin.ignore();
    cout << "Enter product name: ";
    getline(cin, p.name);
    cout << "Enter vendor name: ";
    getline(cin, p.vendor);
    cout << "Enter base price: ";
    cin >> p.basePrice;
    cout << "Enter initial demand score (0-20): ";
    cin >> p.demand;

    p.currentPrice = p.basePrice;

    products.push_back(p);
    priceCache[p.id] = p.currentPrice;
    vendorLookup[p.id] = p.vendor;
    vendorProducts[p.vendor].push_back(p.id);

    cout << "Product added.\n";
}

void updateDemand() {
    int id, delta;
    cout << "Enter product id and demand change (+/-): ";
    cin >> id >> delta;

    for (auto &p : products) {
        if (p.id == id) {
            p.demand = max(0, p.demand + delta);
            cout << "Updated demand for " << p.name
                 << " to " << p.demand << ".\n";
            return;
        }
    }
    cout << "Product id not found.\n";
}

void showAllProducts() {
    if (products.empty()) {
        cout << "No products.\n";
        return;
    }
    cout << "---- All Products ----\n";
    for (auto &p : products) {
        cout << "ID: " << p.id
             << " | " << p.name
             << " | Vendor: " << p.vendor
             << " | Base: " << p.basePrice
             << " | Demand: " << p.demand
             << " | Current: " << p.currentPrice << "\n";
    }
}

// ---------- LOOKUPS ----------
void lookupPrice() {
    int id;
    cout << "Enter product id: ";
    cin >> id;
    if (!priceCache.count(id)) {
        cout << "Product not found in cache.\n";
        return;
    }
    cout << "Current price for id " << id
         << " = " << priceCache[id] << "\n";
}

void lookupVendorProducts() {
    string vendor;
    cin.ignore();
    cout << "Enter vendor name: ";
    getline(cin, vendor);

    if (!vendorProducts.count(vendor) || vendorProducts[vendor].empty()) {
        cout << "No products for this vendor.\n";
        return;
    }

    cout << "Products for vendor " << vendor << ":\n";
    for (int id : vendorProducts[vendor]) {
        cout << "  ID " << id << "\n";
    }
}

// ---------- TRENDING / RANKING ----------
void showTopKTrending() {
    int k;
    cout << "Enter k: ";
    cin >> k;

    if (products.empty()) {
        cout << "No products.\n";
        return;
    }

    priority_queue<Product, vector<Product>, DemandCmp> pq;
    for (auto &p : products) pq.push(p);

    cout << "Top " << k << " trending products (by demand):\n";
    for (int i = 0; i < k && !pq.empty(); ++i) {
        Product p = pq.top(); pq.pop();
        cout << p.id << " | " << p.name
             << " | Demand: " << p.demand
             << " | Price: " << p.currentPrice << "\n";
    }
}

void rankListings() {
    if (products.empty()) {
        cout << "No products to rank.\n";
        return;
    }

    vector<Product> sorted = products;
    // QuickSort/MergeSort via std::sort with custom comparator:
    // higher demand first; if tie, cheaper first.
    sort(sorted.begin(), sorted.end(),
         [](const Product &a, const Product &b) {
             if (a.demand != b.demand)
                 return a.demand > b.demand;
             return a.currentPrice < b.currentPrice;
         });

    cout << "Ranked listings (by demand desc, price asc):\n";
    for (auto &p : sorted) {
        cout << p.id << " | " << p.name
             << " | Demand: " << p.demand
             << " | Price: " << p.currentPrice
             << " | Vendor: " << p.vendor << "\n";
    }
}

// ---------- MAIN MENU ----------
void printMenu() {
    cout << "\n=== Dynamic E-Commerce Pricing Engine ===\n";
    cout << "1. Add product\n";
    cout << "2. Update demand score\n";
    cout << "3. Recompute dynamic prices for current hour\n";
    cout << "4. Show all products\n";
    cout << "5. Lookup current price by product id (hash cache)\n";
    cout << "6. Show products for a vendor (lookup table)\n";
    cout << "7. Show top-k trending products (heap)\n";
    cout << "8. Show ranked listings (QuickSort comparator)\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int choice;
    do {
        printMenu();
        cin >> choice;
        switch (choice) {
            case 1: addProduct(); break;
            case 2: updateDemand(); break;
            case 3: {
                int hour;
                cout << "Enter current hour (0-23): ";
                cin >> hour;
                recomputeAllPrices(hour);
                break;
            }
            case 4: showAllProducts(); break;
            case 5: lookupPrice(); break;
            case 6: lookupVendorProducts(); break;
            case 7: showTopKTrending(); break;
            case 8: rankListings(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
