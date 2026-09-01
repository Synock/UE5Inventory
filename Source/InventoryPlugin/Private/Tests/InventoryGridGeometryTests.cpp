// Copyright 2026 Maximilien (Synock) Guislain

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_WORKER

#include "UI/InventoryGridGeometry.h"
#include "UI/InventoryGridWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryGridNestedGeometryDropTest,
	"InventoryPlugin.UI.InventoryGrid.NestedGeometryDrop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryGridNestedGeometryDropTest::RunTest(const FString& Parameters)
{
	const FGeometry WidgetGeometry = FGeometry::MakeRoot(
		FVector2D(240.0, 200.0), FSlateLayoutTransform(1.0f, FVector2D(100.0, 80.0)));
	const FGeometry BorderGeometry = WidgetGeometry.MakeChild(
		FVector2D(160.0, 160.0), FSlateLayoutTransform(FVector2D(40.0, 20.0)));
	const FGeometry CanvasGeometry = WidgetGeometry.MakeChild(
		FVector2D(160.0, 160.0), FSlateLayoutTransform(FVector2D(40.0, 20.0)));
	const FVector2D VisualInset(8.0, 8.0);
	const FGeometry* GridGeometry = InventoryGridGeometry::ResolveGridGeometry(
		&CanvasGeometry, &BorderGeometry);

	TestTrue(TEXT("Item canvas is preferred over its border"), GridGeometry == &CanvasGeometry);
	const FVector2D PaintOrigin = InventoryGridGeometry::GridOriginInWidgetLocal(
		WidgetGeometry, GridGeometry, VisualInset);
	TestEqual(TEXT("Paint X includes the item visual inset"), PaintOrigin.X, 48.0);
	TestEqual(TEXT("Paint Y includes the item visual inset"), PaintOrigin.Y, 28.0);

	UInventoryGridWidget* GridWidget = NewObject<UInventoryGridWidget>(GetTransientPackage());
	TestNotNull(TEXT("Inventory grid widget can be created"), GridWidget);
	if (!GridWidget)
		return false;

	const FVector2D OneByOneLocal = InventoryGridGeometry::AbsoluteToGridLocal(
		WidgetGeometry, GridGeometry,
		CanvasGeometry.LocalToAbsolute(VisualInset + FVector2D(10.0, 10.0)), VisualInset);
	TestEqual(TEXT("Visually inset cursor X is grid-local"), OneByOneLocal.X, 10.0);
	TestEqual(TEXT("Visually inset cursor Y is grid-local"), OneByOneLocal.Y, 10.0);
	TestEqual(TEXT("Offset 1x1 cursor remains in visual column zero"),
		GridWidget->GetActualTopLeftCorner(OneByOneLocal.X, OneByOneLocal.Y, 1, 1), 0);

	const FVector2D WideItemLocal = InventoryGridGeometry::AbsoluteToGridLocal(
		WidgetGeometry, GridGeometry,
		CanvasGeometry.LocalToAbsolute(VisualInset + FVector2D(39.0, 10.0)), VisualInset);
	TestEqual(TEXT("Offset wide-item cursor remains anchored at visual column zero"),
		GridWidget->GetActualTopLeftCorner(WideItemLocal.X, WideItemLocal.Y, 2, 1), 0);

	const FGeometry* BorderFallback = InventoryGridGeometry::ResolveGridGeometry(nullptr, &BorderGeometry);
	TestTrue(TEXT("Missing canvas falls back to the border"), BorderFallback == &BorderGeometry);

	const FVector2D FallbackLocal = InventoryGridGeometry::AbsoluteToGridLocal(
		WidgetGeometry, InventoryGridGeometry::ResolveGridGeometry(nullptr, nullptr),
		WidgetGeometry.LocalToAbsolute(FVector2D(10.0, 10.0)));
	TestEqual(TEXT("Missing border falls back to widget-local X"), FallbackLocal.X, 10.0);
	TestEqual(TEXT("Missing border falls back to widget-local Y"), FallbackLocal.Y, 10.0);

	return true;
}

#endif
