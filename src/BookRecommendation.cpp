#include "../include/BookRecommendation.h"

#include <algorithm>

void BookRecommendation::addUserBorrowedBook(Patron& userID, Book& book)
{
	// check if user is in the table already
	UnorderedSet<Book>* bookSet = userBorrowedBooks.search(userID.ID);

	// if not in table, add them to the table with a new set containing their book
	if (bookSet == nullptr)
	{
		bookSet = new UnorderedSet<Book>();
		bookSet->insert(book);
		userBorrowedBooks.insert(userID.ID, *bookSet);
	}
	// otherwise, insert book into corresponding set
	else
	{
		bookSet->insert(book);
	}

	// check if book is in the table already
	UnorderedSet<Patron>* patronSet = bookBorrowedByUsers.search(book.ISBN);

	// if not in table, add it to the table with a new set containing the user
	if (patronSet == nullptr)
	{
		patronSet = new UnorderedSet<Patron>();
		patronSet->insert(userID);
		bookBorrowedByUsers.insert(book.ISBN, *patronSet);
	}
	// otherwise, insert user into corresponding set
	else
	{
		patronSet->insert(userID);
	}
}

std::vector<Book> BookRecommendation::getBookRecommendations(const std::string& targetUserID, int numRecommendations, int neighborhoodSize)
{
	// Vector to store book recommendations
	std::vector<Book> bookRecommendations;

	// Fetch neighborhood and their recommended books
	UnorderedSet<std::string> neighborhood = getNeighborhood(targetUserID, neighborhoodSize);
	UnorderedSet<Book> recommendedBooks = getRecommendedBooks(neighborhood, targetUserID);

	// Filter and rank the recommendations
	for (const auto& book : recommendedBooks)
	{
		auto bookUsers = bookBorrowedByUsers.search(book.ISBN);
		int userCount = bookUsers ? bookUsers->size() : 0;

		// Only consider books that are borrowed by enough users
		if (userCount >= numRecommendations)
		{
			bookRecommendations.push_back(book);
		}
	}

	// Sort recommendations based on the number of users who have borrowed them
	std::sort(bookRecommendations.begin(), bookRecommendations.end(), [this](const Book& b1, const Book& b2) -> bool
		{
			auto usersB1 = bookBorrowedByUsers.search(b1.ISBN);
			auto usersB2 = bookBorrowedByUsers.search(b2.ISBN);

			int countB1 = usersB1 ? usersB1->size() : 0;
			int countB2 = usersB2 ? usersB2->size() : 0;

			return countB1 > countB2;
		});

	return bookRecommendations;
}

double BookRecommendation::calculateSimilarity(const std::string& userID1, const std::string& userID2)
{
	// Retrieve the set of borrowed books for both users
	UnorderedSet<Book>* set1 = userBorrowedBooks.search(userID1);
	UnorderedSet<Book>* set2 = userBorrowedBooks.search(userID2);

	// Ensure both users exist and have borrowed books. If not, similarity is 0.
	if (set1 == nullptr || set2 == nullptr || !set1->size() || !set2->size())
	{
		return 0;
	}

	// Count of books borrowed by both users
	double intersection = 0.0;

	// Count books borrowed by both users
	for (const auto& book : *set1)
	{
		// if user2 has it too, then increment count
		if (set2->search((book)))
		{
			intersection += 1;
		}
	}

	// Calculate Jaccard similarity
	// Formula: (Size of Intersection) / (Size of Union)
	return intersection / (set1->size() + set2->size() - intersection);
}

UnorderedSet<std::string> BookRecommendation::getNeighborhood(const std::string& targetUserID, int neighborhoodSize)
{
	// Hashtable to store similarities
	HashTable<std::string, double> similarities;

	// Calculate similarity scores for all users compared to the target user
	// Store similarities by user key into similarity hashtable
	for (const auto& user : userBorrowedBooks)
	{
		if (user->occupied && user->key != targetUserID)
		{
			similarities.insert(user->key, calculateSimilarity(targetUserID, user->key));
		}
	}

	// Unordered set to store neighbor user IDs
	UnorderedSet<std::string> neighbors;

	// Collect the top-scoring users up to the neighborhood size
	for (int i = 0; i < neighborhoodSize; i++)
	{
		std::string mostSimilarUser = "";
		double highestScore = 0.0;

		for (const auto& similarity : similarities)
		{
			if (similarity->value > highestScore)
			{
				mostSimilarUser = similarity->key;
				highestScore = similarity->value;
			}
		}

		// If a user with a positive score is found, add them to neighbors and remove them from similarities
		if (!mostSimilarUser.empty())
		{
			neighbors.insert(mostSimilarUser);
			similarities.remove(mostSimilarUser);
		} else break;
	}

	return neighbors;
}

UnorderedSet<Book> BookRecommendation::getRecommendedBooks(const UnorderedSet<std::string>& neighborhood, const std::string& targetUserID)
{
	// Unordered set to store recommended books
	UnorderedSet<Book> recommendations;

	// Get the list of books the target user has already read
	UnorderedSet<Book>* targetUserBooks = userBorrowedBooks.search(targetUserID);
	// Return empty set if target user has no books;
	if (targetUserBooks == nullptr) return recommendations;

	// Iterate over each user in the neighborhood
	for (const auto& userID : neighborhood)
	{
		// Fetch books for the current user
		UnorderedSet<Book>* neighborBooks = userBorrowedBooks.search(userID);
		// Skip iteration if the neighbor has no books
		if (neighborBooks == nullptr) continue;

		// Add books to recommendations if not already read by the target user
		for (const auto& book : *neighborBooks)
		{
			if (!targetUserBooks->search(book))
			{
				recommendations.insert(book);
			}
		}
	}

	return recommendations;
}