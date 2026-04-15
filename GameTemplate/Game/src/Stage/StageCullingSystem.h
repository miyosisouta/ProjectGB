/**
 * StageCullingSystem.h
 * ステージカリングのシステム
 */
#pragma once


class StaticObject;


class StageCullingSystem
{
public:
	StageCullingSystem();
	~StageCullingSystem();
	
	void Update(std::vector<StaticObject*> staticObjects);
};