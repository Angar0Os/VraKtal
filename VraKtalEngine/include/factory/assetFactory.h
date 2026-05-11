#pragma once
namespace factory 
{
	/*
		All factories should be owned and registered by and in the ressourceManager 
		All fatories will have a public method Create that return the graphics::resource::TRessource and take a graphics::assets::TAsset
			(ig : graphics::resources::Mesh Create(const graphics::assets::Mesh& asset))
		All factories stores ref to what they need to create a a resource
	*/
}