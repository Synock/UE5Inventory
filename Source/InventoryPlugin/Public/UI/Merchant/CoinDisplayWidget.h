#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoinValue.h"
#include "CoinDisplayWidget.generated.h"

/**
 * @class UCoinDisplayWidget
 *
 * Widget for displaying coin values with icons and text.
 * Supports setting values via FCoinValue struct or individual currency amounts.
 */
UCLASS(BlueprintType)
class INVENTORYPLUGIN_API UCoinDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements - Copper
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UImage* CopperIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UTextBlock* CopperText = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// UI Elements - Silver
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UImage* SilverIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UTextBlock* SilverText = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// UI Elements - Gold
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UImage* GoldIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UTextBlock* GoldText = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// UI Elements - Platinum
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UImage* PlatinumIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Coin Display")
	UTextBlock* PlatinumText = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Configurable Icon Textures
	//------------------------------------------------------------------------------------------------------------------

	/** Texture to display for copper coin icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Coin Display|Icons")
	UTexture2D* CopperIconTexture = nullptr;

	/** Texture to display for silver coin icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Coin Display|Icons")
	UTexture2D* SilverIconTexture = nullptr;

	/** Texture to display for gold coin icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Coin Display|Icons")
	UTexture2D* GoldIconTexture = nullptr;

	/** Texture to display for platinum coin icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Coin Display|Icons")
	UTexture2D* PlatinumIconTexture = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Data
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Coin Display")
	int32 CopperPieces = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Coin Display")
	int32 SilverPieces = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Coin Display")
	int32 GoldPieces = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Coin Display")
	int32 PlatinumPieces = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Display Control
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Coin Display")
	bool bShowZeroValues = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Coin Display")
	bool bShowIcons = true;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update all UI elements with current values
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void UpdateDisplay();

	/**
	 * @brief Update visibility of currency elements based on values and settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void UpdateVisibility();

	/**
	 * @brief Apply the configured icon textures to the image widgets
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void ApplyIconTextures();

	/**
	 * @brief Called during widget initialization in both editor and runtime
	 * Queries GameInstance for project-specific coin textures and applies them
	 */
	virtual void NativePreConstruct() override;

public:
	//------------------------------------------------------------------------------------------------------------------
	// Public Interface
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Set the displayed coin value from a FCoinValue struct
	 * @param CoinValue The coin value to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetCoinValue(const FCoinValue& CoinValue);

	/**
	 * @brief Set the copper pieces value
	 * @param Value Number of copper pieces
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetCopperPieces(int32 Value);

	/**
	 * @brief Set the silver pieces value
	 * @param Value Number of silver pieces
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetSilverPieces(int32 Value);

	/**
	 * @brief Set the gold pieces value
	 * @param Value Number of gold pieces
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetGoldPieces(int32 Value);

	/**
	 * @brief Set the platinum pieces value
	 * @param Value Number of platinum pieces
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetPlatinumPieces(int32 Value);

	/**
	 * @brief Set all currency values individually
	 * @param Copper Number of copper pieces
	 * @param Silver Number of silver pieces
	 * @param Gold Number of gold pieces
	 * @param Platinum Number of platinum pieces
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetAllCurrencyValues(int32 Copper, int32 Silver, int32 Gold, int32 Platinum);

	/**
	 * @brief Get the current coin value as a FCoinValue struct
	 * @return The current coin value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Coin Display")
	FCoinValue GetCoinValue() const;

	/**
	 * @brief Clear all displayed values (set to zero)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void Clear();

	/**
	 * @brief Set whether to show zero values
	 * @param bShow If true, currencies with zero value will be shown
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetShowZeroValues(bool bShow);

	/**
	 * @brief Set whether to show currency icons
	 * @param bShow If true, currency icons will be shown
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Coin Display")
	void SetShowIcons(bool bShow);
};

