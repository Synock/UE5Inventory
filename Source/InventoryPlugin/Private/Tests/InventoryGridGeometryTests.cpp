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
	UInventoryGridWidget* GridWidget = NewObject<UInventoryGridWidget>(GetTransientPackage());
	TestNotNull(TEXT("Inventory grid widget can be created"), GridWidget);
	if (!GridWidget)
		return false;

	for (const float LayoutScale : {1.0f, 1.25f, 2.0f})
	{
		const FString ScaleContext = FString::Printf(TEXT(" at %.2fx layout scale"), LayoutScale);
		const FGeometry WidgetGeometry = FGeometry::MakeRoot(
			FVector2D(240.0, 200.0), FSlateLayoutTransform(LayoutScale, FVector2D(100.0, 80.0)));
		const FGeometry BorderGeometry = WidgetGeometry.MakeChild(
			FVector2D(160.0, 160.0), FSlateLayoutTransform(FVector2D(36.0, 16.0)));
		const FGeometry CanvasGeometry = WidgetGeometry.MakeChild(
			FVector2D(160.0, 160.0), FSlateLayoutTransform(FVector2D(40.0, 20.0)));
		// Model the stale desktop/tick geometry seen while a PIE window is resizing. Paint-origin
		// calculation must use CanvasGeometry from the current paint pass, never this transform.
		const FGeometry StaleTickGeometry = FGeometry::MakeRoot(
			FVector2D(160.0, 160.0),
			FSlateLayoutTransform(LayoutScale * 0.8f, FVector2D(240.0, 140.0)));
		const FGeometry* GridGeometry = InventoryGridGeometry::ResolveGridGeometry(
			&CanvasGeometry, &BorderGeometry);

		TestTrue(TEXT("Item canvas is preferred over its border") + ScaleContext,
			GridGeometry == &CanvasGeometry);
		const FVector2D PaintOrigin = InventoryGridGeometry::GridOriginInWidgetLocal(
			WidgetGeometry, GridGeometry);
		TestEqual(TEXT("Paint X starts at the canvas origin") + ScaleContext, PaintOrigin.X, 40.0);
		TestEqual(TEXT("Paint Y starts at the canvas origin") + ScaleContext, PaintOrigin.Y, 20.0);
		const FVector2D StaleOrigin = InventoryGridGeometry::GridOriginInWidgetLocal(
			WidgetGeometry, &StaleTickGeometry);
		TestTrue(TEXT("A stale tick transform would produce a different paint X") + ScaleContext,
			!FMath::IsNearlyEqual(StaleOrigin.X, PaintOrigin.X));
		TestTrue(TEXT("A stale tick transform would produce a different paint Y") + ScaleContext,
			!FMath::IsNearlyEqual(StaleOrigin.Y, PaintOrigin.Y));

		const auto GridLocalAt = [&WidgetGeometry, GridGeometry, &CanvasGeometry](const FVector2D& Position)
		{
			return InventoryGridGeometry::AbsoluteToGridLocal(
				WidgetGeometry, GridGeometry, CanvasGeometry.LocalToAbsolute(Position));
		};

		const FVector2D FirstCellLocal = GridLocalAt(FVector2D(10.0, 10.0));
		TestEqual(TEXT("Painting and hit-testing preserve X") + ScaleContext, FirstCellLocal.X, 10.0);
		TestEqual(TEXT("Painting and hit-testing preserve Y") + ScaleContext, FirstCellLocal.Y, 10.0);
		TestEqual(TEXT("First cell resolves to index zero") + ScaleContext,
			GridWidget->GetActualTopLeftCorner(FirstCellLocal.X, FirstCellLocal.Y, 1, 1), 0);

		const FVector2D LastColumnLocal = GridLocalAt(FVector2D(159.0, 10.0));
		TestEqual(TEXT("Last column resolves correctly") + ScaleContext,
			GridWidget->GetActualTopLeftCorner(LastColumnLocal.X, LastColumnLocal.Y, 1, 1), 3);

		const FVector2D LastCellLocal = GridLocalAt(FVector2D(159.0, 159.0));
		TestEqual(TEXT("Last cell resolves correctly") + ScaleContext,
			GridWidget->GetActualTopLeftCorner(LastCellLocal.X, LastCellLocal.Y, 1, 1), 15);

		const FVector2D WideItemLocal = GridLocalAt(FVector2D(39.0, 10.0));
		TestEqual(TEXT("Wide item remains anchored in column zero") + ScaleContext,
			GridWidget->GetActualTopLeftCorner(WideItemLocal.X, WideItemLocal.Y, 2, 1), 0);

		const FGeometry* BorderFallback = InventoryGridGeometry::ResolveGridGeometry(nullptr, &BorderGeometry);
		TestTrue(TEXT("Missing canvas falls back to the border") + ScaleContext,
			BorderFallback == &BorderGeometry);

		const FVector2D FallbackLocal = InventoryGridGeometry::AbsoluteToGridLocal(
			WidgetGeometry, InventoryGridGeometry::ResolveGridGeometry(nullptr, nullptr),
			WidgetGeometry.LocalToAbsolute(FVector2D(10.0, 10.0)));
		TestEqual(TEXT("Missing border falls back to widget-local X") + ScaleContext, FallbackLocal.X, 10.0);
		TestEqual(TEXT("Missing border falls back to widget-local Y") + ScaleContext, FallbackLocal.Y, 10.0);
	}

	return true;
}

#endif
