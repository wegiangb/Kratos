//Authors: M.A. Celigueta and S. Latorre (CIMNE)
//   Date: July 2015

#if !defined(DEM_D_CONICAL_DAMAGE_CL_H_INCLUDED)
#define DEM_D_CONICAL_DAMAGE_CL_H_INCLUDED

#include <string>
#include <iostream>
#include "DEM_discontinuum_constitutive_law.h"

namespace Kratos {
    
    class SphericParticle;

    class DEM_D_Conical_damage : public DEMDiscontinuumConstitutiveLaw {
    
    public:

        KRATOS_CLASS_POINTER_DEFINITION(DEM_D_Conical_damage);

        DEM_D_Conical_damage() {}

        ~DEM_D_Conical_damage() {}

        void Initialize(const ProcessInfo& r_process_info);

        void SetConstitutiveLawInProperties(Properties::Pointer pProp) const;
        
        std::string GetTypeOfLaw();

        DEMDiscontinuumConstitutiveLaw::Pointer Clone() const;

        void InitializeContact(SphericParticle* const element1, SphericParticle* const element2, const double equiv_radius, const double equiv_young, const double equiv_shear, const double indentation);
        
        void DamageContact(SphericParticle* const element1, SphericParticle* const element2, double equiv_radius, const double equiv_young, const double equiv_shear, const double indentation, const double normal_contact_force);
        
        void InitializeContactWithFEM(SphericParticle* const element, DEMWall* const wall, const double effective_radius, const double equiv_young, const double equiv_shear, const double indentation, const double ini_delta = 0.0);
        
        void DamageContactWithFEM(SphericParticle* const element, DEMWall* const wall, double effective_radius, const double equiv_young, const double equiv_shear, const double indentation, const double normal_contact_force, const double ini_delta = 0.0);

        void CalculateForces(const ProcessInfo& r_process_info,
                             const double OldLocalElasticContactForce[3],
                             double LocalElasticContactForce[3],
                             double LocalDeltDisp[3],
                             double LocalRelVel[3],            
                             double indentation,
                             double previous_indentation,
                             double ViscoDampingLocalContactForce[3],
                             double& cohesive_force,
                             SphericParticle* element1,
                             SphericParticle* element2,
                             bool& sliding);

        void CalculateForcesWithFEM(ProcessInfo& r_process_info,
                                    const double OldLocalElasticContactForce[3],
                                    double LocalElasticContactForce[3],
                                    double LocalDeltDisp[3],
                                    double LocalRelVel[3],            
                                    double indentation,
                                    double previous_indentation,
                                    double ViscoDampingLocalContactForce[3],
                                    double& cohesive_force,
                                    SphericParticle* const element,
                                    DEMWall* const wall,
                                    bool& sliding);
                
        double CalculateNormalForce(const double indentation);
        
        double CalculateCohesiveNormalForce(SphericParticle* const element1,
                                            SphericParticle* const element2,
                                            const double indentation);

        double CalculateCohesiveNormalForceWithFEM(SphericParticle* const element,
                                                   DEMWall* const wall,
                                                   const double indentation);

        template <class NeighbourClassType>
        void CalculateTangentialForceWithNeighbour(const double normal_contact_force,
                                                   const double OldLocalElasticContactForce[3],
                                                   double LocalElasticContactForce[3],
                                                   double ViscoDampingLocalContactForce[3],
                                                   const double LocalDeltDisp[3],            
                                                   bool& sliding,
                                                   SphericParticle* const element,
                                                   NeighbourClassType* const neighbour,
                                                   const double equiv_radius,
                                                   const double equiv_young,
                                                   double indentation,
                                                   double previous_indentation,
                                                   double& ActualTotalShearForce, 
                                                   double& MaximumAdmisibleShearForce);

        void CalculateViscoDampingForce(double LocalRelVel[3],
                                        double ViscoDampingLocalContactForce[3],
                                        SphericParticle* const element1,
                                        SphericParticle* const element2);

        void CalculateViscoDampingForceWithFEM(double LocalRelVel[3],
                                        double ViscoDampingLocalContactForce[3],
                                        SphericParticle* const element,
                                        DEMWall* const wall);

        void CalculateElasticEnergyDEM(double& elastic_energy,
                                       double indentation,
                                       double LocalElasticContactForce[3]);

        void CalculateInelasticFrictionalEnergyDEM(double& inelastic_frictional_energy,
                                                   double& ActualTotalShearForce,
                                                   double& MaximumAdmisibleShearForce);
        
        void CalculateInelasticViscodampingEnergyDEM(double& inelastic_viscodamping_energy,
                                                     double ViscoDampingLocalContactForce[3],
                                                     double LocalDeltDisp[3]);
        
        void CalculateElasticEnergyFEM(double& elastic_energy,
                                       double indentation,
                                       double LocalElasticContactForce[3]);

        void CalculateInelasticFrictionalEnergyFEM(double& inelastic_frictional_energy,
                                                   double& ActualTotalShearForce,
                                                   double& MaximumAdmisibleShearForce);
        
        void CalculateInelasticViscodampingEnergyFEM(double& inelastic_viscodamping_energy,
                                                     double ViscoDampingLocalContactForce[3],
                                                     double LocalDeltDisp[3]);
                            
    private:

        friend class Serializer;
        
        virtual void save(Serializer& rSerializer) const {
            KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, DEMDiscontinuumConstitutiveLaw)
                    //rSerializer.save("MyMemberName",myMember);
        }

        virtual void load(Serializer& rSerializer) {
            KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, DEMDiscontinuumConstitutiveLaw)
                    //rSerializer.load("MyMemberName",myMember);
        }
        
    }; //class DEM_D_Conical_damage

} /* namespace Kratos.*/
#endif /* DEM_D_CONICAL_DAMAGE_CL_H_INCLUDED  defined */