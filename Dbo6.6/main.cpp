#include <iostream>
#include <Windows.h>
#include <vector>
#include <algorithm>

#include <Wt/Dbo/Dbo.h>
#include<Wt/Dbo/backend/Postgres.h>
#pragma execution_character_set("utf-8")

class book;
class stock;
class sale;
class shop;

class publisher {
public:
	std::string name = "";
	Wt::Dbo::collection<Wt::Dbo::ptr<book>> books;

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, name, "name");
		Wt::Dbo::hasMany(a, books, Wt::Dbo::ManyToOne, "publisher");
	}

};

class book {
public:
	std::string title = "";
	Wt::Dbo::ptr<publisher> publisher;
	Wt::Dbo::collection<Wt::Dbo::ptr<stock>> stocks;


	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, title, "title");
		Wt::Dbo::belongsTo(a, publisher, "publisher");
		Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "book");
	}
};

class stock {
public:
	int count = 0;
	Wt::Dbo::ptr<book> book;
	Wt::Dbo::ptr<shop> shop;
	Wt::Dbo::collection<Wt::Dbo::ptr<sale>> sales;

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, count, "count");
		Wt::Dbo::belongsTo(a, shop, "shop");
		Wt::Dbo::belongsTo(a, book, "book");
		Wt::Dbo::hasMany(a, sales, Wt::Dbo::ManyToOne, "stock");
	}
};

class sale {
public:
	int price = 0;
	int date_sale = 0;
	int count = 0;
	Wt::Dbo::ptr<stock> stock;

	template<class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, price, "price");
		Wt::Dbo::field(a, date_sale, "date_sale");
		Wt::Dbo::field(a, count, "count");
		Wt::Dbo::belongsTo(a, stock, "stock");
	}
};

class shop {
public:
	std::string name = "";
	Wt::Dbo::collection<Wt::Dbo::ptr<stock>> stocks;

	template <class Action>
	void persist(Action& a)
	{
		Wt::Dbo::field(a, name, "name");
		Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "shop");
	}
};

auto add_publisher(std::string name) {

	return std::move(std::unique_ptr<publisher>(new publisher{ name }));
}

auto add_book(std::string name) {

	return std::move(std::unique_ptr<book>(new book{ name }));
}

auto add_stock(int count) {

	return std::move(std::unique_ptr<stock>(new stock{ count }));
}

auto add_shop(std::string name) {

	return std::move(std::unique_ptr<shop>(new shop{ name }));
}

int main() {

	try {

		setlocale(LC_ALL, "Russian");
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);
		setvbuf(stdout,nullptr,_IOFBF,1000);

		std::string conn = "host=localhost"
			" port=5432 dbname=forneto "
			"user=postgres password=123";

		//using namespace Wt::Dbo::backend;

		auto connection = std::make_unique<Wt::Dbo::backend::Postgres>(conn);

		Wt::Dbo::Session session;
		session.setConnection(std::move(connection));

		session.mapClass<publisher>("publisher");
		session.mapClass<book>("book");
		session.mapClass<shop>("shop");
		session.mapClass<stock>("stock");
		session.mapClass<sale>("sale");
		try{
			session.createTables();
		}catch(...){}

		Wt::Dbo::Transaction transaction{ session };

		//auto moscow = session.add(add_publisher("moscow"));
		//auto windows = session.add(add_publisher("windows"));

		//auto book1 = session.add(add_book("Kremlin"));
		//auto book2 = session.add(add_book("Boris Pasternak"));
		//auto book3 = session.add(add_book("forms"));

		//auto stock1 = session.add(add_stock(10));
		//auto stock2 = session.add(add_stock(50));
		//auto stock3 = session.add(add_stock(100));

		//auto shop1 = session.add(add_shop("online bookhop"));
		//auto shop2 = session.add(add_shop("moscow's books"));

		//book1.modify()->publisher = moscow;
		//book2.modify()->publisher = moscow;
		//book3.modify()->publisher = windows;

		//stock1.modify()->book = book1;
		//stock2.modify()->book = book2;
		//stock3.modify()->book = book3;

		//shop1.modify()->stocks.insert(stock3);
		//shop2.modify()->stocks.insert(stock2);
		//shop2.modify()->stocks.insert(stock1);

		std::string name_publisher;
		std::cout << "Input publisher's name: "; std::cin >> name_publisher;

		Wt::Dbo::ptr<publisher> publisher_ = session.find<publisher>().where("name=?").bind(name_publisher);

		typedef Wt::Dbo::collection<Wt::Dbo::ptr<book>> books;
		books Books = session.find<book>().where("publisher_id=?").bind(publisher_.id());
		
		typedef Wt::Dbo::collection<Wt::Dbo::ptr<stock>> stocks;
		std::vector<Wt::Dbo::ptr<shop>> result;

		for (auto it = Books.begin(); it != Books.end(); ++it) {
			stocks Stocks = session.find<stock>().where("book_id = ?").bind(it->id());
			for (auto it1 = Stocks.begin(); it1 != Stocks.end(); ++it1) {
				Wt::Dbo::ptr<shop> res = session.find<shop>().where("id = ?").bind((*it1)->shop);
				if (!(std::find(result.begin(), result.end(), res) != result.end())) {
					result.push_back(res);
				}
			}
		}

		std::cout << "Shops with \"" << name_publisher << "\" publisher:\n";
		for (auto it = result.begin(); it != result.end(); ++it) {
			std::cout << (*it)->name << "\n";
		}

		if (result.empty()) std::cout << "no one found...\n";

		transaction.commit();
		std::cout << "All ok!\n";
	}
	catch (const Wt::Dbo::Exception &e) {
		std::cout << e.what() << std::endl;
	}
}