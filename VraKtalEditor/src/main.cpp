#include <iostream>
#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <graphics/renderer.h>
#include <graphics/renderPass/gBufferPass.h>
#include <graphics/resources/object/light.h>
#include <graphics/assets/material.h>

#include <core/manager/ressourceManager.h>
#include <core/manager/assetManager.h>

#include <core/manager/sceneManager.h>

#include <scene/system/systemManager.h>
#include <scene/system/systems/meshSystem.h>

#include <loaders/meshLoader.h>
#include <factory/meshFactory.h>

#include <factory/materialFactory.h>
#include "imGuiWindows.h"
#include "utils/yamlParser.h"
#include <core/input/input.h>

#include <core/manager/ressourceManager.h>

#include <scene/system/systemManager.h>
#include <scene/system/systems/meshSystem.h>

#include <scene/scene.h>
#include <scene/timeline/components/mesh.h>
#include <scene/timeline/components/light.h>

#include <vraktal.h>

#ifdef VRAKTAL_EDITOR
    #pragma comment(lib, "VraKtalEngine_Debug.lib")
    #include <core/gpu/imguiContext.h>
#else
    #pragma comment(lib, "VraKtalEngine.lib")
#endif // VRAKTAL_EDITOR
#include <scene/system/systems/lightSystem.h>
#include <memory>
#include <graphics/resources/object/mesh.h>
#include <graphics/assets/mesh.h>
#include <graphics/resources/material.h>


class App
{
public:
    App(Vraktal& vraktal){
        m_vraktal = &vraktal;
        vraktal.GetInput().AddAction("CloseApp");
        vraktal.GetInput().BindActionKey({ input::Key::ESCAPE }, "CloseApp");
        vraktal.GetInput().BindActionCallback<App, &App::CloseApp>("CloseApp", this, input::KeyState::Press);

        vraktal.GetInput().AddAction("SpawnVikingRoom");
        vraktal.GetInput().BindActionKey({ input::Key::F }, "SpawnVikingRoom");
        vraktal.GetInput().BindActionCallback<App, &App::SpawnVikingRoom>("SpawnVikingRoom", this, input::KeyState::Press);

        EntityID SceneID = m_vraktal->GetSceneManager().CreateScene("Debug");

        vraktal.GetSceneManager().GetScene(SceneID).RegisterComponentStorage<timeline::MeshInstance>();
        vraktal.GetSceneManager().GetScene(SceneID).RegisterComponentStorage<timeline::Light>();

        /*
            Donc pour creer une ressource il faut : 
            Creer son asset puis creer la ressource a partir de l'asset
            
        */
        
        //puisqu'on a pas d'asset pour l'instant on va les creer
        graphics::assets::Mesh meshAsset;
        meshAsset.path = "assets/models/viking_room.obj";
        loaders::MeshLoader::LoadMeshFromDisk("assets/models/viking_room.obj", meshAsset);
        vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Mesh>("assets/models/viking_room.obj", meshAsset);
     
        vraktal.GetRessourceManager().CreateRessource<graphics::resources::Mesh>(vraktal.GetAssetManager().GetAsset<graphics::assets::Mesh>(vraktal.GetAssetManager().GetAssetID<graphics::assets::Mesh>("assets/models/viking_room.obj")));
        graphics::resources::Mesh& meshRessource = vraktal.GetRessourceManager().GetResource<graphics::resources::Mesh>(vraktal.GetAssetManager().GetAssetID<graphics::assets::Mesh>("assets/models/viking_room.obj"));
        
        graphics::assets::Material Sand;
        Sand.name = "Sand";
        Sand.SetTexture("assets/textures/extracted_textures/diffuse_sand.jpg.png", "albedo");
        Sand.SetTexture("assets/textures/extracted_textures/normal_sand.png.png", "normal");
        uint32_t idSand = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("Sand", Sand);
        graphics::assets::Material& matSand = vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idSand);

        uint32_t MatId = vraktal.GetRessourceManager().CreateRessource<graphics::resources::Material>(matSand); //Material factory is not init correctly maybe make all factory static
        meshRessource.materialIds.push_back(MatId);

    }
    ~App() {};

    bool ShouldClose() const { return bSouldCloseApp; }
    void CloseApp() {
        bSouldCloseApp = true;
        std::cout << "Close App Action Triggered" << std::endl;
    };

    void SpawnVikingRoom() {
        std::cout << "Spawn Viking Room Action Triggered" << std::endl;
        timeline::MeshInstance timelineMesh;
        timelineMesh.assetID = m_vraktal->GetAssetManager().GetAssetID<graphics::assets::Mesh>("assets/models/cave.obj");
        timelineMesh.temp_properties.transform = glm::rotate(glm::mat4(1), glm::radians(270.0f), { 1,0,0 });
        timelineMesh.temp_properties.transform = glm::scale(timelineMesh.temp_properties.transform, glm::vec3(10, 10, 10));
        m_vraktal->GetSceneManager().GetScene(m_vraktal->GetSceneManager().GetSceneID("Debug")).CreateEntity<timeline::MeshInstance>(timelineMesh);
    }

    void LoadAssetsDebug(graphics::Renderer& _renderer , core::gpu::Device& _device , Vraktal& vraktal)
    {
        graphics::assets::Mesh meshAsset;
        meshAsset.path = "assets/models/cave.obj";
        loaders::MeshLoader::LoadMeshFromDisk("assets/models/cave.obj", meshAsset);
        vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Mesh>("assets/models/cave.obj", meshAsset);

        vraktal.GetRessourceManager().CreateRessource<graphics::resources::Mesh>(vraktal.GetAssetManager().GetAsset<graphics::assets::Mesh>(vraktal.GetAssetManager().GetAssetID<graphics::assets::Mesh>("assets/models/cave.obj")));
        graphics::resources::Mesh& meshRessource = vraktal.GetRessourceManager().GetResource<graphics::resources::Mesh>(vraktal.GetAssetManager().GetAssetID<graphics::assets::Mesh>("assets/models/cave.obj"));


        auto* matLayout = _renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();
        uint32_t idRocktill = 0;
        uint32_t idwatreplant = 0;
        uint32_t idfoliage = 0;
        uint32_t idCaveFoliage = 0;
        uint32_t idCave = 0;
        uint32_t idgrass = 0;
        uint32_t idCaveIndoor = 0;
        uint32_t idCaveBole = 0;
        uint32_t idRocks = 0;
        uint32_t idStones = 0;
        uint32_t idskeleton = 0;
        uint32_t idfetich = 0;
        uint32_t idGrassTransition = 0;

        {
            graphics::assets::Material Rocktill;
            Rocktill.name = "Rocktill";
            Rocktill.SetTexture("assets/textures/extracted_textures/diffus_rocktill_02.jpg.png", "albedo");
            Rocktill.SetTexture("assets/textures/extracted_textures/normal_rocktill_02.png.png", "normal");
            Rocktill.roughness = 0.8;
            Rocktill.metallic = 0.0;
            idRocktill = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("Rocktill", Rocktill);


            graphics::assets::Material watreplant;
            watreplant.name = "watreplant";
            watreplant.SetTexture("assets/textures/extracted_textures/diffus_plane_watreplant.tga.png", "albedo");
            watreplant.SetTexture("assets/textures/extracted_textures/normal_plane_watreplant.png.png", "normal");
            Rocktill.roughness = 0.88;
            Rocktill.metallic = 0.0;
            idwatreplant = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("watreplant", watreplant);

            graphics::assets::Material foliage;
            foliage.name = "foliage";
            foliage.SetTexture("assets/textures/extracted_textures/diffus_foliage_03_copy.tga.png", "albedo");
            foliage.SetTexture("assets/textures/extracted_textures/normal_foliage_03.png.png", "normal");
            foliage.roughness = 0.88;
            foliage.metallic = 0.0;
            idfoliage = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("foliage", foliage);

            graphics::assets::Material CaveFoliage;
            CaveFoliage.name = "Cave Foliage";
            CaveFoliage.SetTexture("assets/textures/extracted_textures/diffuse_cave_foliage.tga.png", "albedo");
            CaveFoliage.SetTexture("assets/textures/extracted_textures/normal_cave_foliage.png.png", "normal");
            CaveFoliage.roughness = 0.88;
            CaveFoliage.metallic = 0.0;
            idCaveFoliage = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("CaveFoliage", CaveFoliage);

            graphics::assets::Material Cave;
            Cave.name = "Cave";
            Cave.SetTexture("assets/textures/extracted_textures/diffuse_cave.jpg.png", "albedo");
            Cave.SetTexture("assets/textures/extracted_textures/normal_cave.png.png", "normal");
            Cave.roughness = 0.88;

            idCave = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("Cave", Cave);

            graphics::assets::Material grass;
            grass.name = "grass";
            grass.SetTexture("assets/textures/extracted_textures/diffuse_grass.jpg.png", "albedo");
            grass.SetTexture("assets/textures/extracted_textures/normal_grass.png.png", "normal");
            grass.roughness = 0.88;

            idgrass = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("grass", grass);

            graphics::assets::Material CaveIndoor;
            CaveIndoor.name = "Cave indoor";
            CaveIndoor.SetTexture("assets/textures/extracted_textures/diffuse_cave_indoor.jpg.png", "albedo");
            CaveIndoor.SetTexture("assets/textures/extracted_textures/normal_cave_indoor.png.png", "normal");


            idCaveIndoor = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("CaveIndoor", CaveIndoor);

            graphics::assets::Material CaveBole;
            CaveBole.name = "Cave bole";
            CaveBole.SetTexture("assets/textures/extracted_textures/diffuse_cave_bole.jpg.png", "albedo");
            CaveBole.SetTexture("assets/textures/extracted_textures/normal_cave_bole.png.png", "normal");


            idCaveBole = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("CaveBole", CaveBole);

            graphics::assets::Material Rocks;
            Rocks.name = "Rocks";
            Rocks.SetTexture("assets/textures/extracted_textures/diffus_rocks.jpg.png", "albedo");
            Rocks.SetTexture("assets/textures/extracted_textures/normal_rocks.png.png", "normal");


            idRocks = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("Rocks", Rocks);

            graphics::assets::Material Stones;
            Stones.name = "Stones";
            Stones.SetTexture("assets/textures/extracted_textures/diffuse_cave_stones.jpg.png", "albedo");
            Stones.SetTexture("assets/textures/extracted_textures/normal_cave_stones.png.png", "normal");


            idStones = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("Stones", Stones);

            graphics::assets::Material skeleton;
            skeleton.name = "skeleton";
            skeleton.SetTexture("assets/textures/extracted_textures/diffuse_skeleton.jpg.png", "albedo");
            skeleton.SetTexture("assets/textures/extracted_textures/normal_skeleton.png.png", "normal");


            idskeleton = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("skeleton", skeleton);

            graphics::assets::Material fetich;
            fetich.name = "fetich";
            fetich.SetTexture("assets/textures/extracted_textures/diffus_fetich.jpg.png", "albedo");
            fetich.SetTexture("assets/textures/extracted_textures/normal_fetich.png.png", "normal");


            idfetich = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("fetich", fetich);

            graphics::assets::Material GrassTransition;
            GrassTransition.name = "grass transition";
            GrassTransition.SetTexture("assets/textures/extracted_textures/diffus_grass-transition_clairiere.jpg.png", "albedo");



            idGrassTransition = vraktal.GetAssetManager().AddExistingAsset<graphics::assets::Material>("GrassTransition", GrassTransition);
        }
        

            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idRocktill));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idwatreplant));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idfoliage));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idCaveFoliage));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idCave));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idgrass));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idCaveIndoor));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idCaveBole));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idStones));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idskeleton));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idfetich));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idGrassTransition));
            vraktal.GetRessourceManager().CreateRessource_ptr<graphics::resources::Material>  (vraktal.GetAssetManager().GetAsset<graphics::assets::Material>(idRocks));
       
            meshRessource.materialIds.push_back(idRocktill);
            meshRessource.materialIds.push_back(idRocktill);
            meshRessource.materialIds.push_back(idwatreplant);
            meshRessource.materialIds.push_back(idfoliage);
            meshRessource.materialIds.push_back(idfoliage);
            meshRessource.materialIds.push_back(idCaveFoliage);
            meshRessource.materialIds.push_back(idCave);
            meshRessource.materialIds.push_back(idgrass);
            meshRessource.materialIds.push_back(idCaveIndoor);
            meshRessource.materialIds.push_back(idCaveBole);
            meshRessource.materialIds.push_back(idCaveFoliage);
            meshRessource.materialIds.push_back(idRocks);
            meshRessource.materialIds.push_back(idRocks);
            meshRessource.materialIds.push_back(idStones);
            meshRessource.materialIds.push_back(idskeleton);
            meshRessource.materialIds.push_back(idfetich);
            meshRessource.materialIds.push_back(idGrassTransition);
    }

private:
    bool bSouldCloseApp = false;
    Vraktal* m_vraktal;
};


int main()
{

#ifndef VRAKTAL_EDITOR
    Vraktal vraktal(800, 600, "VraKtal");

    //Load all things here ? 
#else
    Vraktal vraktal(1200, 720, "VraKtal Editor");

    ImGuiWindows imGuiWindows = ImGuiWindows(vraktal);
    vraktal.GetDevice().GetImGuiContext()->BindPrepareDrawData([&]()
        {
            imGuiWindows.DrawImGui();
        });
    
    App app(vraktal);
    app.LoadAssetsDebug(vraktal.GetRenderer(), vraktal.GetDevice(), vraktal);
    app.SpawnVikingRoom();

#endif //VRAKTAL_EDITOR

    while (!vraktal.ShouldClose())
    {
        vraktal.Update();

        if (!vraktal.BeginFrame())
            continue;

#ifdef VRAKTAL_EDITOR
        imGuiWindows.Update();
#else
        vraktal.Render();
#endif // VRAKTAL_EDITOR

        vraktal.EndFrame();
    }

    vraktal.Cleanup();

    return 0;
} 