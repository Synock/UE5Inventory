#pragma once

#include "CoreMinimal.h"
#include "Items/InventoryItemBase.h"
#include "Items/Interfaces/InventoryItemBookInterface.h"
#include "InventoryItemBook.generated.h"

/**
 * Concrete readable item. Authored entirely in the editor as a Data Asset.
 *
 * Implements IInventoryItemBookInterface — all widget / HUD systems that use
 * IInventoryHUDInterface::DisplayBookText work with this class automatically.
 *
 * Single-page use (letter, note):
 *   Fill Pages with one FBookPage entry; BookTitle is the window title.
 *
 * Multi-page use (book, tome, journal):
 *   Fill Pages with one FBookPage per page.  PageNumber is auto-assigned
 *   sequentially from array order so the designer does not need to set it manually.
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYPLUGIN_API UInventoryItemBook : public UInventoryItemBase, public IInventoryItemBookInterface
{
	GENERATED_BODY()

public:
	/** Title displayed in the book widget header and window chrome. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Book")
	FText BookTitle;

	/** Author attribution (optional — empty = anonymous). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Book")
	FText Author;

	/**
	 * Page content. Single-element array = letter / note.
	 * PageNumber fields are automatically overwritten on retrieval to match array order.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Book", meta = (TitleProperty = "Title"))
	TArray<FBookPage> Pages;

	// ----------------------------------------------------------------------------------------------------------------
	// IInventoryItemBookInterface
	// ----------------------------------------------------------------------------------------------------------------

	virtual FText GetBookTitle_Implementation() const override { return BookTitle; }
	virtual FText GetBookAuthor_Implementation() const override { return Author; }

	virtual TArray<FBookPage> GetBookPages_Implementation() const override
	{
		// Auto-assign sequential page numbers matching array order
		TArray<FBookPage> Result = Pages;
		for (int32 i = 0; i < Result.Num(); ++i)
			Result[i].PageNumber = i + 1;
		return Result;
	}

	virtual int32 GetPageCount_Implementation() const override { return Pages.Num(); }

	// CanActivate and IsConsumedOnUse defaults (always readable, never consumed)
	// are already provided by IInventoryItemBookInterface.
};

