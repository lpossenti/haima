/* -*- c++ -*- (enableMbars emacs c++ mode) */
/*======================================================================
    "Mixed Finite Element Methods for Coupled 3D/1D Fluid Problems"
        Course on Advanced Programming for Scientific Computing
                      Politecnico di Milano
                          A.Y. 2014-2015
                  
                Copyright (C) 2015 Domenico Notaro
======================================================================*/
/*! 
	@file   param3d1d.hpp
	@author Domenico Notaro <domenico.not@gmail.com>
	@date   January 2016.
	@brief  Definition of the aux class for physical parameters.
	@details 
	Assemble the dimensionless parameters of the coupled 3D/1D model:
	- Radius @f$R'(s)@f$,
	- Tissue permeability @f$\kappa_t@f$,
	- Vessel wall permeability @f$Q(s)@f$,
	- Vessel bed permeability @f$\kappa_v(s)@f$.

	being @f$s\in\Lambda@f$ the arc-lenght over the vessel network.
	\note @f$\kappa_t@f$ is assumed to be constant.

	\ingroup input
 */
 
#ifndef M3D1D_PARAM3D1D_HPP_
#define M3D1D_PARAM3D1D_HPP_

//#define M3D1D_VERBOSE_

#include <mesh1d.hpp>    // import_network_radius
#include <utilities.hpp> // compute_radius
#include <c_mesh1d.hpp> //rasm_curve_parameter 

namespace getfem {

//! Class to handle the physical parameter of the coupled 3D/1D model
struct param3d1d {

	// Dimensional physical parameters (microcirc applications)
	//! Average interstitial pressure [Pa]
	scalar_type P_; 
	//! Characteristic flow speed in the capillary bed [m/s]
	scalar_type U_; 
	//! Characteristic length of the problem [m]
	scalar_type d_; 
	//! Hydraulic conductivity of the interstitium [m^2]
	scalar_type k_; 
	//! Viscosity of the blood [kg/ms]
	scalar_type mu_v_; 	
	//! Viscosity of the interstitial fluid [kg/ms]
	scalar_type mu_t_; 
	//! Hydraulic conductivity of the capillary walls [m^2 s/kg]
	scalar_type Lp_;
        //! Hydraulic conductivity of the lymphatic vessels [m s/kg] (linear case)
        scalar_type Lp_LF_;
	//! Coefficient A of the lymphatic sigmoid [s-1]
        scalar_type A_LF_;
	//! Coefficient B of the lymphatic sigmoid [s-1]
        scalar_type B_LF_;
	//! Coefficient C of the lymphatic sigmoid [Pa]
        scalar_type C_LF_;
	//! Coefficient D of the lymphatic sigmoid [Pa]
        scalar_type D_LF_;
	// Dimensionless physical parameters (test-cases)
	//! Dimensionless average radius of the vessel network
	scalar_type Rav_;
	//! Dimensionless radii of the vessel branches
	vector_type R_;
        //! vectorial Hydraulic conductivity of the capillary walls [m^2 s/kg]
        vector_type Lp_vec_;
        //! vectorial Dimensionless reflection coefficient
        vector_type sigma_vec_;
         //! vectorial Young modulus of the vessel wall
        vector_type E_vec_;
	//! Dimensionless thickness of the vessel branches
	vector_type thick_;
	//! Dimensionless radius of the vessel branches (small vector)
	//GR vector_type Ri_;
	//! Dimensionless areas of the cross sections
	vector_type CSarea_;                                             // change eli
	//! Dimensionless perimeters of the cross sections
	vector_type CSper_;                                              // change eli
	//! Dimensionless conductivity of the tissue
	vector_type kt_;
	//! Dimensionless conductivity of the vessel wall
	vector_type Q_;
	//! Dimensionless conductivity of the vessel bed
	vector_type kv_;
	//! Dimensionless conductivity of the vessel bed divided by branch
	//GR vector_type kvi_;
        //! Dimensionless hydraulic conductivity of the lymphatic vessels (linear case)
        vector_type Q_LF_;
	// Dimensionless parameter A of the lymphatic sigmoid 
        scalar_type QLF_A_;
	// Dimensionless parameter B of the lymphatic sigmoid 
        scalar_type QLF_B_;
	// Dimensionless parameter C of the lymphatic sigmoid
        scalar_type QLF_C_;
	// Dimensionless parameter D of the lymphatic sigmoid
        scalar_type QLF_D_;
	// Dimensionless plasma oncotic pressure
	scalar_type pi_v_;
	// Dimensionless interstitial oncotic pressure
	scalar_type pi_t_;
	// Dimensionless reflection coefficient
	scalar_type sigma_;
	// Utils
	//! File .param
	ftool::md_param FILE_;
	//! Finite Element Method for tissue data
	getfem::mesh_fem mf_datat_;
	//! Finite Element Method for vessel data
	getfem::mesh_fem mf_datav_;
	//! Mesh tangent versor X component
	vector<vector_type> lambdax_;
	//! Mesh tangent versor Y component
	vector<vector_type> lambday_;
	//! Mesh tangent versor Z component
	vector<vector_type> lambdaz_;	
	//! Mesh curvature
	vector<vector_type> Curv_;
	//! Young modulus of the vessel wall
	scalar_type E_;
	//! Poisson modulus of the vessel wall
	scalar_type nu_;
	/*! Order of velocity profile 
		s=\frac{1}{\gamma} (\gamma + 2) (1+(\frac{r}{R})^{\gamma})
		if \gamma=2 we have poiseuille profile
	*/
	scalar_type Gamma_;
	// Methods
	//! Build the arrays of Dimensionless parameters
	void build(ftool::md_param & fname, 
			const getfem::mesh_fem & mf_datat,
			const getfem::mesh_fem & mf_datav,
			const vector<getfem::mesh_fem> & mf_datavi
			) 
	{
		FILE_ = fname;
		mf_datat_ = mf_datat;
		mf_datav_ = mf_datav;
		size_type dof_datat = mf_datat_.nb_dof();
		size_type dof_datav = mf_datav_.nb_dof();
		size_type n_branch= mf_datavi.size();
		 
                bool IMPORT_RADIUS = FILE_.int_value("IMPORT_RADIUS");
                bool IMPORT_LP = FILE_.int_value("IMPORT_LP");
                bool IMPORT_SIGMA = FILE_.int_value("IMPORT_SIGMA");
                bool IMPORT_E = FILE_.int_value("IMPORT_E");
		bool NONDIM_PARAM  = FILE_.int_value("TEST_PARAM");
		bool EXPORT_PARAM  = FILE_.int_value("EXPORT_PARAM");
		bool LINEAR_LYMPHATIC_DRAINAGE = FILE_.int_value("LINEAR_LYMPHATIC_DRAINAGE");
		bool IMPORT_CURVE = FILE_.int_value("CURVE_PROBLEM");

		// Check
		//GR gmm::resize(Ri_, n_branch);
		if (IMPORT_RADIUS)
			GMM_ASSERT1(NONDIM_PARAM == 0,
				"try to import non constant (dimensionless) radius:" 
				"please insert dimensional parameters");
		#ifdef M3D1D_VERBOSE_
		cout << "  Assembling dimensionless radius R'... "   << endl;
		#endif
		if (!IMPORT_RADIUS) { 	/* case R' = const */

			if (NONDIM_PARAM) // we assume it is already non-dimensional
				Rav_ = FILE_.real_value("RADIUS", "Vessel average radius");
			else // to be non-dimensionalized
				Rav_ = FILE_.real_value("RADIUS", "Vessel average radius")/FILE_.real_value("d");
			R_.assign(dof_datav, Rav_);
			//GR Ri_.assign(n_branch, Rav_);
		} else { 				/* case R' = R'(s) */
			std::string RFILE = FILE_.string_value("RFILE"); 
			cout << "  Importing radius values from file " << RFILE << " ..." << endl;
			std::ifstream ist(RFILE);
			if (!ist) cerr << "impossible to read from file " << RFILE << endl;
			import_network_radius(R_, ist, mf_datav_);//GR import_network_radius(R_,Ri_, ist, mf_datav_);;
			/*for (size_type i=1; i < 9; i++ ){
				R_[i] = R_[i-1] + 0.00007;
			}
			for (size_type i=9; i < 17; i++ ){
				R_[i] = R_[i-1] - 0.00007;
			}*/
			std::string THICKFILE = FILE_.string_value("THICKFILE"); 
			cout << "  Importing thickness values from file " << THICKFILE << " ..." << endl;
			std::ifstream istt(THICKFILE);
			if (!istt) { 
				cerr << "impossible to read from file " << THICKFILE << endl;
				gmm::resize(thick_, mf_datav_.nb_dof()); 
				for( size_type i=0; i < R_.size(); i++) thick_[i] = R_[i] * 0.2; // if file not found, set all arterioles
			}
			else import_network_radius(thick_, istt, mf_datav_);//GR import_network_radius(R_,Ri_, ist, mf_datav_);;

			gmm::resize(CSper_, dof_datav);
			gmm::resize(CSarea_, dof_datav);
			for (size_type i=0; i < dof_datav ; i++){
				CSarea_[i] = pi * R_[i] * R_[i];
				CSper_[i] = 2 *pi *R_[i];	
				//thick_[i] = R_[i] *0.08;
				//cout << " R_["<<i<<"] = "<< R_[i]<<endl;
				//cout << " CSarea_["<<i<<"] = "<< CSarea_[i]<<endl;
			}
		}
		
		
                if(IMPORT_LP){
                        std::string FILE = FILE_.string_value("LPFILE"); 
                        cout << "  Importing LP values from file " << FILE << " ..." << endl;
                        std::ifstream ist(FILE);
                        if (!ist) cerr << "impossible to read param from file " << FILE << endl;
                        import_network_radius(Lp_vec_, ist, mf_datav_);
                }
                if(IMPORT_SIGMA){
                        std::string FILE = FILE_.string_value("SIGMAFILE"); 
                        cout << "  Importing SIGMA values from file " << FILE << " ..." << endl;
                        std::ifstream ist(FILE);
                        if (!ist) cerr << "impossible to read param from file " << FILE << endl;
                        import_network_radius(sigma_vec_, ist, mf_datav_);
                }
                if(IMPORT_E){
                        std::string FILE = FILE_.string_value("EFILE"); 
                        cout << "  Importing Young modulus values from file " << FILE << " ..." << endl;
                        std::ifstream ist(FILE);
                        if (!ist) cerr << "impossible to read param from file " << FILE << endl;
                        import_network_radius(E_vec_, ist, mf_datav_);
                }

		if(!IMPORT_CURVE){
			#ifdef M3D1D_VERBOSE_
			cout<<"CURVE NOT IMPORTED, THE PROBLEM IS CONSIDERED LINEAR FOR ALL BRANCHES\n\n";
			#endif

			vector_type lx_temp,ly_temp,lz_temp;
			std::ifstream ifst(FILE_.string_value("MESH_FILEV","1D points file"));
			GMM_ASSERT1(ifst.good(), "impossible to read from file " << FILE_.string_value("MESH_FILEV","1D points file"));
			asm_tangent_versor(ifst, lx_temp,ly_temp,lz_temp);
			Curv_.resize(n_branch);
			lambdax_.resize(n_branch);
			lambday_.resize(n_branch);
			lambdaz_.resize(n_branch); 

			for(size_type b=0;b<n_branch;++b){
				size_type dofi=mf_datavi[b].nb_dof();
				Curv_[b].resize(dofi); Curv_[b].clear();
				Curv_[b].assign(dofi, 0.0);
				
				gmm::resize(lambdax_[b],dofi);
				gmm::resize(lambday_[b],dofi);
				gmm::resize(lambdaz_[b],dofi);
				
				lambdax_[b].assign(dofi,lx_temp[b]);
				lambday_[b].assign(dofi,ly_temp[b]);
				lambdaz_[b].assign(dofi,lz_temp[b]);
				
			}
		} else {
			rasm_curve_parameter(mf_datavi,Curv_,lambdax_,lambday_,lambdaz_);
			for(size_type b=0;b<n_branch;++b) //GR
			 gmm::scaled(Curv_[b],1.0/FILE_.real_value("d")); //GR FG//gmm::scaled(Curv_[b],1.0);
		}


		#ifdef M3D1D_VERBOSE_
		cout << "  Assembling dimensionless permeabilities kt, Q, kv ... "   << endl;
		#endif
		if (NONDIM_PARAM) {
                        std::cout << " Import NON dimensional " << std::endl;
			// Import dimensionless params from FILE_
			scalar_type ktval = FILE_.real_value("Kt"); 
			scalar_type Qval  = FILE_.real_value("Q"); 
			scalar_type kvval = FILE_.real_value("Kv");
			pi_t_ = FILE_.real_value("pi_t_adim");
			pi_v_ = FILE_.real_value("pi_v_adim");
			sigma_ = FILE_.real_value("sigma");
			Gamma_ = 2.0;
			if(!LINEAR_LYMPHATIC_DRAINAGE)
			{
                        QLF_A_ = FILE_.real_value("QLF_A", "Dimensionless parameter A of lymphatic drainage");
                        QLF_B_ = FILE_.real_value("QLF_B", "Dimensionless parameter B of lymphatic drainage");
                        QLF_C_ = FILE_.real_value("QLF_C", "Dimensionless parameter C of lymphatic drainage");
                        QLF_D_ = FILE_.real_value("QLF_D", "Dimensionless parameter D of lymphatic drainage");
 			Q_LF_.assign(dof_datat, 0);
			}
			else
			{
                        scalar_type QLFval  = FILE_.real_value("Q_LF");
                        Q_LF_.assign(dof_datat, QLFval);
			}
			// Fill the data arrays
			  kt_.assign(dof_datat,  ktval);
			  kv_.assign(dof_datav,  kvval);
			  //GR kvi_.assign(n_branch,kvval);
			   Q_.assign(dof_datav,   Qval);

		} 
		else {
			// Import dimensional params from FILE_
                        std::cout << " Import dimensional " << std::endl;
			P_  = FILE_.real_value("P", "average interstitial pressure [Pa]"); 
			U_  = FILE_.real_value("U", "characteristic flow speed in the capillary bed [m/s]"); 
			d_  = FILE_.real_value("d", "characteristic length of the problem [m]"); 
			k_  = FILE_.real_value("k", "permeability of the interstitium [m^2]"); 
			mu_v_ = FILE_.real_value("mu_v", "blood viscosity [kg/ms]"); 
			mu_t_ = FILE_.real_value("mu_t", "interstitial fluid viscosity [kg/ms]"); 
			pi_t_ = FILE_.real_value("Pi_t", "interstitial oncotic pressure [Pa]"); 
			pi_v_ = FILE_.real_value("Pi_v", "fluid oncotic pressure [Pa]"); 
			sigma_ = FILE_.real_value("sigma", "reflection coefficient [-]"); 
			Gamma_= FILE_.real_value("Gamma", "Order of velocity profile in the vessels");
			Lp_   = FILE_.real_value("Lp", "permeability of the vessel walls [m^2 s/kg]");
			E_    = FILE_.real_value("E", "Young modulus of the vessel wall [Pa]");
			nu_   = FILE_.real_value("nu", "Poisson modulus of the vessel wall [-]");
			if(LINEAR_LYMPHATIC_DRAINAGE)
			{
                        Lp_LF_ = FILE_.real_value("Lp_LF", "permeability of lymphatic vessels [(m s) / kg] ");
                        Q_LF_.assign(dof_datat, Lp_LF_*P_*d_/U_);
			}
			// Compute the dimenless params
			kt_.assign(dof_datat, k_/mu_t_*P_/U_/d_);
			pi_t_ = pi_t_/P_;
			pi_v_ = pi_v_/P_; 
                        if (!IMPORT_LP)
                            for (auto r : R_){ // C++11-only!
                                    kv_.emplace_back(pi/2.0/(Gamma_+2.0)/mu_v_*P_*d_/U_*r*r*r*r);
                                    Q_.emplace_back(2.0*pi*Lp_*P_/U_*r);
                            }
                        else
                        {   std::cout << " creating kv_ and Q depending on r and lp variable"<<std::endl;
                            for (int i =0; i<R_.size(); i++){ // C++11-only!
                                double r= R_[i];
                                double Lp= Lp_vec_[i];
                                    kv_.emplace_back(pi/2.0/(Gamma_+2.0)/mu_v_*P_*d_/U_*r*r*r*r);
                                    Q_.emplace_back(2.0*pi*Lp*P_/U_*r);
                            }
                        }
			/*for(auto r: Ri_){
				kvi_.emplace_back(pi/2.0/(Gamma_+2.0)/mu_v_*P_*d_/U_*r*r*r*r);
			} GR*/
			// Fixed Point Method for Lymphatic System (Sigmoid)
			if(!LINEAR_LYMPHATIC_DRAINAGE)
			{
                        A_LF_ = FILE_.real_value("A_LF", "First Coefficient (A) of the lymphatic flow [s-1] ");
                        B_LF_ = FILE_.real_value("B_LF", "Second Coefficient (B) of the lymphatic flow [s-1]");
                        C_LF_ = FILE_.real_value("C_LF", "Third Coefficient (C) of the lymphatic flow [Pa]");
                        D_LF_ = FILE_.real_value("D_LF", "Fourth Coefficient (D) of the lymphatic flow [Pa]");
			QLF_A_ = A_LF_/U_*d_;	
			QLF_B_ = B_LF_/U_*d_;
			QLF_C_ = C_LF_/P_;
			QLF_D_ = D_LF_/P_;
 			Q_LF_.assign(dof_datat, 0);
			}
		}
		// Check values
		GMM_ASSERT1(kt_[0] != 0, "wrong tissue conductivity (kt>0 required)"); 
		GMM_ASSERT1(kv_[0] != 0, "wrong vessel bed conductivity (kv>0 required)");
		if (Q_[0] == 0) cout << "Warning: uncoupled problem (Q=0)" << endl;
		
		if (EXPORT_PARAM){
			std::string ODIR = FILE_.string_value("OutputDir","OutputDirectory");
			getfem::vtk_export exp(ODIR+"radius.vtk");
			exp.exporting(mf_datav_);
			exp.write_mesh();
			exp.write_point_data(mf_datav_, R_, "R");
			getfem::vtk_export expQ(ODIR+"conductivity.vtk");
			expQ.exporting(mf_datav_);
			expQ.write_mesh();
			expQ.write_point_data(mf_datav_, Q_, "Q");
		}

	}

	//! Saving the curved parameters during the initialisation
	void get_curve(
		vector<vector_type> & Curv, 
		vector<vector_type> & lambdax, 
		vector<vector_type> & lambday, 
		vector<vector_type> & lambdaz
	)
	{
		Curv_=Curv;
		lambdax_=lambdax;
		lambday_=lambday;
		lambdaz_=lambdaz;
	}

	//! Get the radius at a given dof
	inline scalar_type R  (size_type i) { return R_[i];  } const //! Get the radius at a given branch //GR inline scalar_type Ri  (size_type i) { return Ri_[i];  } const
	//! Get the Cross Section area at a given dof
	inline scalar_type CSarea(size_type i) { return CSarea_[i]; } const
	//! Get the Cross Section perimeter at a given dof
	inline scalar_type CSper(size_type i) { return CSper_[i]; } const
	//! Get the thickness of the wall at a given dof
	inline scalar_type thick(size_type i) { return thick_[i]; } const
	//! Get the tissue permeability at a given dof
	inline scalar_type kt (size_type i) { return kt_[i]; } const
	//! Get the vessel bed permeability at a given dof
	inline scalar_type kv (size_type i) { return kv_[i]; } const//! Get the vessel bed permeability at a given dof //GR inline scalar_type kvi (size_type i) { return kvi_[i]; } const
	//! Get the vessel wall permeability at a given dof
	inline scalar_type Q  (size_type i) { return Q_[i];  } const
	//! Get the radius at a given mesh_region
        scalar_type R  (const getfem::mesh_im & mim, const size_type rg) { 
                return compute_radius(mim, mf_datav_, R_, rg); // Ri_[rg]; // compute_radius(mim, mf_datav_, R_, rg);
        }
        //! Get the sigma at a given mesh_region
        scalar_type sigma  (const getfem::mesh_im & mim, const size_type rg) { 
                return compute_radius(mim, mf_datav_, sigma_vec_, rg); // Ri_[rg]; // compute_radius(mim, mf_datav_, R_, rg);
        }
        //! Get the LP at a given mesh_region
        scalar_type Lp  (const getfem::mesh_im & mim, const size_type rg) { 
                return compute_radius(mim, mf_datav_, Lp_vec_, rg); // Ri_[rg]; // compute_radius(mim, mf_datav_, R_, rg);
        }
        //! Get the young modulus at a given mesh_region
        scalar_type E  (const getfem::mesh_im & mim, const size_type rg) { 
                return compute_radius(mim, mf_datav_, E_vec_, rg); // Ri_[rg]; // compute_radius(mim, mf_datav_, R_, rg);
        }
	//! Get the Cross section area at a given mesh_region
	scalar_type CSarea  (const getfem::mesh_im & mim, const size_type rg) { 
		return compute_radius(mim, mf_datav_, CSarea_, rg);
	}
	//! Get the Cross Section perimeter at a given mesh_region
	scalar_type CSper  (const getfem::mesh_im & mim, const size_type rg) { 
		return compute_radius(mim, mf_datav_, CSper_, rg);
	}
	//! Get the vessel bed permeability at a given mesh_region
	scalar_type kv  (const getfem::mesh_im & mim, const size_type rg) { 
		return compute_radius(mim, mf_datav_, kv_, rg); // kvi_[rg]; // compute_radius(mim, mf_datav_, kv_, rg);
	}
	//! Get the radius
	vector_type & R (void) { return R_; }
	//void replace_r ( vector_type R_new){ R_ = R_new; }
	void replace_r ( scalar_type R_new, size_type i){ R_[i] = R_new; }
	//! Get the Cross Section area
	vector_type & CSarea(void) { return CSarea_; }
	//! Modify the values of cross section area
	void replace_area ( vector_type area_new){ CSarea_ = area_new; }
	void replace_area ( scalar_type area_new, size_type i){ CSarea_[i] = area_new; }
	//! Get the Cross Section perimeter
	vector_type & CSper(void) { return CSper_; }
	//! Modify the values of cross section perimeter
	void replace_per ( vector_type per_new){ CSper_ = per_new; }
	void replace_per ( scalar_type per_new, size_type i){ CSper_[i] = per_new; }
	//! Get the thickness of vessel wall
	vector_type & thick (void) { return thick_; }
	//! Get the vessel wall permeabilities
	vector_type & Q (void) { return Q_; }
	//! Get the vessel bed permeabilities
	vector_type & kv (void) { return kv_; }
        //! Get the lymphatic vessels permeability
        inline scalar_type Q_LF (size_type i) { return Q_LF_[i]; } const
	//! Get the coefficient of lymphatic vessel
	inline scalar_type QLF_a  (void) { return QLF_A_; } const
	inline scalar_type QLF_b  (void) { return QLF_B_;  } const
	inline scalar_type QLF_c  (void) { return QLF_C_;  } const
	inline scalar_type QLF_d  (void) { return QLF_D_;  } const
	//! Get the interstitial oncotic pressure
	inline scalar_type pi_t  (void) { return pi_t_; } const
	//! Get the plasma oncotic pressure
	inline scalar_type pi_v  (void) { return pi_v_; } const
	//! Get the reflection coefficient
	inline scalar_type sigma  (void) { return sigma_; } const
	//! Get the esponent of velocity profile
	inline scalar_type Gamma (void) { return Gamma_;} const
	//! Get Young modulus of the vessel wall
	inline scalar_type E (void) { return E_;} const
	//! Get Poisson modulus of the vessel wall
	inline scalar_type nu (void) { return nu_;} const
	//! Get vessel tangent versor x component
	vector<vector_type> & lambdax (void) { return lambdax_; }
	//! Get vessel tangent versor y component
	vector<vector_type> & lambday (void) { return lambday_; }
	//! Get vessel tangent versor z component
	vector<vector_type> & lambdaz (void) { return lambdaz_; }
	//! Get vessel tangent versor x component for branch i
	vector_type & lambdax (size_type i) { return lambdax_[i]; }
	//! Get vessel tangent versor y component for branch i
	vector_type & lambday (size_type i) { return lambday_[i]; }
	//! Get vessel tangent versor z component for branch i
	vector_type & lambdaz (size_type i) { return lambdaz_[i]; }
	//! Get vessel curvature
	vector<vector_type> & Curv (void) { return Curv_; }
	//! Get vessel curvature for branch i
	vector_type & Curv (size_type i) { return Curv_[i]; }
	//! Get vessel curvature for branch i in position j
	scalar_type & Curv (size_type i, size_type j) { return Curv_[i][j]; }
	


	//! Overloading of the output operator
	friend std::ostream & operator << (
		std::ostream & out, const param3d1d & param
		)
	{ 
		out << "--- PHYSICAL PARAMS ------" << endl;
		out << "  R'     : "                << param.R_[0] << endl; 
		out << "  kappat : "                << param.kt_[0] << endl; 
		out << "  Q      : "                << param.Q_[0] << endl; 
                out << "  kappav : "                << param.kv_[0] << endl;
		out << "  Gamma  : "                << param.Gamma_ << endl; 
		out << "--------------------------" << endl;

		return out;            
	}
}; /* end of class */

} /* end of namespace */

#endif
