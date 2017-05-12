// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>

/**
 * 
 */
class PONGGAME_API FRelay
{
public:
	FRelay();
	~FRelay();

    std::string Act(const std::string& state);

private:

    std::string host;
};
