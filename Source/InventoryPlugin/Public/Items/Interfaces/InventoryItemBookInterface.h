#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemActivatableInterface.h"
#include "InventoryItemBookInterface.generated.h"

/**
 * Page data for multi-page readable items.
 */
USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FBookPage
{
	GENERATED_BODY()

	/** Page number (1-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	int32 PageNumber = 1;

	/** Page title (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	FText Title;

	/** Page content (supports rich text) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book", meta=(MultiLine=true))
	FText Content;

	/** Optional image for this page */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	TObjectPtr<UTexture2D> Image;

	/** Optional background texture for this page */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	TObjectPtr<UTexture2D> BackgroundTexture;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryItemBookInterface : public UInventoryItemActivatableInterface
{
	GENERATED_BODY()
};

/**
 * Interface for readable book items (books, tomes, journals, etc.).
 * Inherits from IInventoryItemActivatableInterface to provide activation behavior.
 *
 * Books are activatable items that:
 * - Open a reading UI when activated (doesn't consume the item)
 * - Can have multiple pages with rich text and images
 * - Can track read status per player
 * - Can grant lore achievements when read
 *
 * Examples: Story book, skill tome, lore journal, quest letter
 *
 * Activation behavior:
 * - CanActivate() -> Always true (books can always be read)
 * - IsConsumedOnUse() -> False (books aren't destroyed when read)
 */
class INVENTORYPLUGIN_API IInventoryItemBookInterface : public IInventoryItemActivatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the title of this book.
	 * @return Book title for UI display
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	FText GetBookTitle() const;
	virtual FText GetBookTitle_Implementation() const
	{
		return {};
	}

	/**
	 * Get all pages of this book.
	 * @return Array of pages (single-page books return array with 1 element)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	TArray<FBookPage> GetBookPages() const;
	virtual TArray<FBookPage> GetBookPages_Implementation() const
	{
		return {};
	}

	/**
	 * Get the number of pages in this book.
	 * @return Total page count
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	int32 GetPageCount() const;
	virtual int32 GetPageCount_Implementation() const
	{
		return 1;
	}

	/**
	 * Get a specific page by number.
	 * @param PageNumber Page to retrieve (1-based)
	 * @param OutPage The page data (if found)
	 * @return True if page exists
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	bool GetPageByNumber(int32 PageNumber, FBookPage& OutPage) const;
	virtual bool GetPageByNumber_Implementation(int32 PageNumber, FBookPage& OutPage) const
	{
		TArray<FBookPage> Pages = Execute_GetBookPages(Cast<UObject>(this));
		for (const FBookPage& Page : Pages)
		{
			if (Page.PageNumber == PageNumber)
			{
				OutPage = Page;
				return true;
			}
		}
		return false;
	}

	/**
	 * Get simple text content for single-page books.
	 * Convenience method for simple books/letters with one page.
	 * @return Content text
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	FText GetSimpleContent() const;
	virtual FText GetSimpleContent_Implementation() const
	{
		TArray<FBookPage> Pages = Execute_GetBookPages(Cast<UObject>(this));
		if (Pages.Num() > 0)
		{
			return Pages[0].Content;
		}
		return FText::GetEmpty();
	}

	/**
	 * Get the author of this book.
	 * @return Author name (empty if anonymous)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	FText GetBookAuthor() const;
	virtual FText GetBookAuthor_Implementation() const
	{
		return FText::GetEmpty();
	}

	/**
	 * Books can always be activated (read).
	 */
	virtual bool CanActivate_Implementation() const override
	{
		// Books can always be read
		return true;
	}

	/**
	 * Books are never consumed when read.
	 */
	virtual bool IsConsumedOnUse_Implementation() const override
	{
		return false;
	}
};
