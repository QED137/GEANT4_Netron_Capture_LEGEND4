/*#include "NeuDetectorConstruction.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"

NeuDetectorConstruction::NeuDetectorConstruction() {}

NeuDetectorConstruction::~NeuDetectorConstruction() {}

G4VPhysicalVolume* NeuDetectorConstruction::Construct()
{
    // Get materials
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");

    // Define the world volume
    G4double worldSize = 1.0 * m;
    G4Box* worldBox = new G4Box("World", worldSize, worldSize, worldSize);
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "World");
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(
        nullptr,               // No rotation
        G4ThreeVector(),       // Centered at origin
        worldLog,              // Logical volume
        "World",               // Name
        nullptr,               // No mother volume
        false,                 // No boolean operation
        0                      // Copy number
    );

    return worldPhys;
}
*/

#include "NeuDetectorConstruction.hh"

#include "DetectorMessenger.hh"

#include "G4Box.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalConstants.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4SolidStore.hh"
#include "G4Sphere.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

NeuDetectorConstruction::NeuDetectorConstruction()
{
  fRadius = 30 * cm;
  fWorldSize = 1.1 * fRadius;
  DefineMaterials();
  SetMaterial("Water_ts");
  fDetectorMessenger = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

NeuDetectorConstruction::~NeuDetectorConstruction()
{
  delete fDetectorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* NeuDetectorConstruction::Construct()
{
  return ConstructVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void NeuDetectorConstruction::DefineMaterials()
{
  // specific element name for thermal neutronHP
  // (see G4ParticleHPThermalScatteringNames.cc)

  G4int ncomponents, natoms;

  // pressurized water
  G4Element* H = new G4Element("TS_H_of_Water", "H", 1., 1.0079 * g / mole);
  G4Element* O = new G4Element("Oxygen", "O", 8., 16.00 * g / mole);
  G4Material* H2O = new G4Material("Water_ts", 1.000 * g / cm3, ncomponents = 2, kStateLiquid,
                                   593 * kelvin, 150 * bar);
  H2O->AddElement(H, natoms = 2);
  H2O->AddElement(O, natoms = 1);
  H2O->GetIonisation()->SetMeanExcitationEnergy(78.0 * eV);

  // heavy water
  G4Isotope* H2 = new G4Isotope("H2", 1, 2);
  G4Element* D = new G4Element("TS_D_of_Heavy_Water", "D", 1);
  D->AddIsotope(H2, 100 * perCent);
  G4Material* D2O = new G4Material("HeavyWater", 1.11 * g / cm3, ncomponents = 2, kStateLiquid,
                                   293.15 * kelvin, 1 * atmosphere);
  D2O->AddElement(D, natoms = 2);
  D2O->AddElement(O, natoms = 1);

  // graphite
  G4Isotope* C12 = new G4Isotope("C12", 6, 12);
  G4Element* C = new G4Element("TS_C_of_Graphite", "C", ncomponents = 1);
  C->AddIsotope(C12, 100. * perCent);
  G4Material* graphite = new G4Material("graphite", 2.27 * g / cm3, ncomponents = 1, kStateSolid,
                                        293 * kelvin, 1 * atmosphere);
  graphite->AddElement(C, natoms = 1);

  // NE213
  G4Material* ne213 = new G4Material("NE213", 0.874 * g / cm3, ncomponents = 2);
  ne213->AddElement(H, 9.2 * perCent);
  ne213->AddElement(C, 90.8 * perCent);

  // example of vacuum
  fWorldMat = new G4Material("Galactic", 1, 1.01 * g / mole, universe_mean_density, kStateGas,
                             2.73 * kelvin, 3.e-18 * pascal);

  /// G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* NeuDetectorConstruction::MaterialWithSingleIsotope(G4String name, G4String symbol,
                                                            G4double density, G4int Z, G4int A)
{
  // define a material from an isotope
  //
  G4int ncomponents;
  G4double abundance, massfraction;

  G4Isotope* isotope = new G4Isotope(symbol, Z, A);

  G4Element* element = new G4Element(name, symbol, ncomponents = 1);
  element->AddIsotope(isotope, abundance = 100. * perCent);

  G4Material* material = new G4Material(name, density, ncomponents = 1);
  material->AddElement(element, massfraction = 100. * perCent);

  return material;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* NeuDetectorConstruction::ConstructVolumes()
{
  // Cleanup old geometry
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // World
  //
  G4Box* sWorld = new G4Box("World",  // name
                            fWorldSize, fWorldSize, fWorldSize);  // dimensions

  G4LogicalVolume* lWorld = new G4LogicalVolume(sWorld,  // shape
                                                fWorldMat,  // material
                                                "World");  // name

  fPWorld = new G4PVPlacement(0,  // no rotation
                              G4ThreeVector(),  // at (0,0,0)
                              lWorld,  // logical volume
                              "World",  // name
                              0,  // mother volume
                              false,  // no boolean operation
                              0);  // copy number

  // Absorber
  //
  G4Sphere* sAbsor = new G4Sphere("Absorber",  // name
                                  0., fRadius, 0., twopi, 0., pi);  // dimensions

  fLAbsor = new G4LogicalVolume(sAbsor,  // shape
                                fMaterial,  // material
                                fMaterial->GetName());  // name

  new G4PVPlacement(0,  // no rotation
                    G4ThreeVector(),  // at (0,0,0)
                    fLAbsor,  // logical volume
                    fMaterial->GetName(),  // name
                    lWorld,  // mother  volume
                    false,  // no boolean operation
                    0);  // copy number
  PrintParameters();

  // always return the root volume
  //
  return fPWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void NeuDetectorConstruction::PrintParameters()
{
  G4cout << "\n The Absorber is " << G4BestUnit(fRadius, "Length") << " of " << fMaterial->GetName()
         << "\n \n"
         << fMaterial << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void NeuDetectorConstruction::SetMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial = G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);

  if (pttoMaterial) {
    fMaterial = pttoMaterial;
    if (fLAbsor) {
      fLAbsor->SetMaterial(fMaterial);
    }
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  }
  else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterial : " << materialChoice
           << " not found" << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void NeuDetectorConstruction::SetRadius(G4double value)
{
  fRadius = value;
  fWorldSize = 1.1 * fRadius;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}
