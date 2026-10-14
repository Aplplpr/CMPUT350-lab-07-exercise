#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "entity_manager.h"
#include "registry.h"

int main() {
    // Test registry
    Registry<int> r;
    assert(r.size() == 0 && !r.valid(0));

    int x = 10;
    assert(r.store(x) == 0);   // lvalue
    assert(r.store(20) == 1);  // rvalue
    assert(r.store(30) == 2);
    assert(r.size() == 3 && !r.valid(3));

    r[1] = 99;  // operator[] returns a reference
    assert(r[1] == 99);
    const Registry<int>& cr = r;
    assert(cr[0] == 10);

    // Removed tickets are recycled before storage grows
    r.remove(1);
    assert(r.size() == 2 && !r.valid(1));
    assert(r.store(40) == 1 && r[1] == 40);
    assert(r.store(50) == 3);

    r.reserve(100);  // does not change size
    assert(r.size() == 4);

    Registry<bool> rb;  // vector<bool> pitfall
    size_t b = rb.store(true);
    assert(rb[b]);

    Registry<std::string> rs;
    std::string s = "hello";
    size_t a = rs.store(s);                  // copy
    assert(s == "hello" && rs[a] == "hello");
    rs.store(std::string("hi"));             // move
    rs.remove(a);
    assert(rs.store("world") == a && rs[a] == "world");

    // remove must release the old resource
    Registry<std::shared_ptr<int>> rp;
    auto p = std::make_shared<int>(42);
    size_t id = rp.store(p);
    assert(p.use_count() == 2);
    rp.remove(id);
    assert(p.use_count() == 1);


    EntityManager<int, float, bool> em;
    assert(!em.validEntity(0));

    EntityID e = em.createEntity();
    EntityID e2 = em.createEntity();
    assert(e == 0 && e2 == 1);
    assert(em.validEntity(e) && !em.validEntity(99));

    assert(!em.hasComponent<int>(e));
    em.addComponent(e, 5);
    assert(em.hasComponent<int>(e));
    assert(!em.hasComponent<float>(e) && !em.hasComponent<bool>(e));
    assert(em.getComponent<int>(e) == 5);

    em.getComponent<int>(e) = 7;  // returns a reference
    assert(em.getComponent<int>(e) == 7);

    em.addComponent(e, true);
    float f = 2.5f;
    em.addComponent(e, f);  // lvalue
    assert(em.getComponent<bool>(e) && em.getComponent<float>(e) == 2.5f);

    // Entities do not affect each other
    assert(!em.hasComponent<int>(e2));
    const int ci = 3;
    em.addComponent(e2, ci);  // const lvalue
    assert(em.getComponent<int>(e2) == 3 && em.getComponent<int>(e) == 7);

    std::cout << "All tests passed.\n";
    
    return 0;
}