#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_DEBUG_SPHERE(Location) if (GetWorld()) { \
		DrawDebugSphere(GetWorld(), Location, 25.f, 12, FColor::Red, true); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_SPHERE_SingleFrame(Location) if (GetWorld()) { \
		DrawDebugSphere(GetWorld(), Location, 25.f, 12, FColor::Red, false, -1); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_LINE(StartLocation, EndLocation) if (GetWorld()) { \
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true,  -1.f, 0, 1.f); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_LINE_SingleFrame(StartLocation, EndLocation) if (GetWorld()) { \
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false,  -1.f, 0, 1.f); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_POINT(Location) if (GetWorld()) { \
		DrawDebugPoint(GetWorld(), Location, 15.f, FColor::Red, true); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_POINT_SingleFrame(Location) if (GetWorld()) { \
		DrawDebugPoint(GetWorld(), Location, 15.f, FColor::Red, false, -1); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_VECTOR(StartLocation, EndLocation) if (GetWorld()) { \
		DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 25.f, FColor::Red, true); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}

#define DRAW_DEBUG_VECTOR_SingleFrame(StartLocation, EndLocation) if (GetWorld()) { \
		DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 25.f, FColor::Red, false, -1.f); \
	} else { \
		UE_LOG(LogTemp, Warning, TEXT("GetWorld() is null!")); \
	}