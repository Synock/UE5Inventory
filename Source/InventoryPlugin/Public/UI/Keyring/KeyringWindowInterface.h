#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KeyringWindowInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UKeyringWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for any widget that acts as a keyring window.
 *
 * Implemented by both UKeyringWidget (plugin inner widget) and UKeyringWindow (game wrapper).
 * IInventoryHUDInterface's C++ defaults dispatch all keyring lifecycle calls through this
 * interface, keeping HUD logic game-agnostic.
 *
 * No refresh method is provided — the keyring list is kept in sync via
 * UKeyringComponent::KeyringChangedDelegate bound in UKeyringWidget::InternalSetup().
 */
class INVENTORYPLUGIN_API IKeyringWindowInterface
{
	GENERATED_BODY()

public:
	/** Make the keyring window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Keyring")
	void ShowKeyringWindow();
	virtual void ShowKeyringWindow_Implementation() {}

	/** Hide the keyring window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Keyring")
	void HideKeyringWindow();
	virtual void HideKeyringWindow_Implementation() {}

	/** Returns true if the keyring window is currently visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Keyring")
	bool IsKeyringWindowVisible() const;
	virtual bool IsKeyringWindowVisible_Implementation() const { return false; }
};

