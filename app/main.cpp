#include <iostream>
#include <mysqlx/xdevapi.h>
#include "db/session_pool.h"
using ::std::cout;
using ::std::endl;
using namespace ::mysqlx;
using ppic::db::SessionPoolOption;
using ppic::db::SessionPool;
int main(int argc, const char* argv[])
{
    SessionPoolOption option;
    option.FromEnv().set_capacity(16);
    ppic::db::SessionPoolSingleton::instance()->InitPool(option);
    try {
    auto sess = ppic::db::SessionPoolSingleton::instance()->ObtainSession();
    cout <<"Session accepted, creating collection..." <<endl;
    Schema sch= sess->getSchema("ppic_test");
    Collection coll= sch.createCollection("c1", true);
    cout <<"Inserting documents..." <<endl;
    coll.remove("true").execute();
    {
        Result add;
        add= coll.add(R"({ "name": "foo", "age": 1 })").execute();
        std::vector<string> ids = add.getGeneratedIds();
        cout <<"- added doc with id: " << ids[0] <<endl;
        add= coll.add(R"({ "name": "bar", "age": 2, "toys": [ "car", "ball" ] })")
                .execute();
        if (ids.size() != 0)
        cout <<"- added doc with id: " << ids[0] <<endl;
        else
        cout <<"- added doc" <<endl;
        add= coll.add(R"({
        "name": "baz",
            "age": 3,
        "date": { "day": 20, "month": "Apr" }
        })").execute();
        if (ids.size() != 0)
        cout <<"- added doc with id: " << ids[0] <<endl;
        else
        cout <<"- added doc" <<endl;
        add= coll.add(R"({ "_id": "myuuid-1", "name": "foo", "age": 7 })")
                .execute();
        ids = add.getGeneratedIds();
        if (ids.size() != 0)
        cout <<"- added doc with id: " << ids[0] <<endl;
        else
        cout <<"- added doc" <<endl;
    }
    cout <<"Fetching documents..." <<endl;
    DocResult docs = coll.find("age > 1 and name like 'ba%'").execute();
    DbDoc doc = docs.fetchOne();
    for (int i = 0; doc; ++i, doc = docs.fetchOne())
    {
        cout <<"doc#" <<i <<": " <<doc <<endl;
        for (Field fld : doc)
        {
        cout << " field `" << fld << "`: " <<doc[fld] << endl;
        }
        string name = doc["name"];
        cout << " name: " << name << endl;
        if (doc.hasField("date") && Value::DOCUMENT == doc.fieldType("date"))
        {
        cout << "- date field" << endl;
        DbDoc date = doc["date"];
        for (Field fld : date)
        {
            cout << "  date `" << fld << "`: " << date[fld] << endl;
        }
        string month = doc["date"]["month"];
        int day = date["day"];
        cout << "  month: " << month << endl;
        cout << "  day: " << day << endl;
        }
        if (doc.hasField("toys") && Value::ARRAY == doc.fieldType("toys"))
        {
        cout << "- toys:" << endl;
        for (auto toy : doc["toys"])
        {
            cout << "  " << toy << endl;
        }
        }
        cout << endl;
    }
    cout <<"Done!" <<endl;
    ppic::db::SessionPoolSingleton::instance()->ReleaseSession(sess);
    }
    catch (const mysqlx::Error &err)
    {
    cout <<"ERROR: " <<err <<endl;
    return 1;
    }
    catch (std::exception &ex)
    {
    cout <<"STD EXCEPTION: " <<ex.what() <<endl;
    return 1;
    }
    catch (const char *ex)
    {
    cout <<"EXCEPTION: " <<ex <<endl;
    return 1;
    }
}