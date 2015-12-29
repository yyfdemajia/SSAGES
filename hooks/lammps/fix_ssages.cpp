#include "fix_ssages.h"
#include "atom.h"
#include <iostream>

using namespace SSAGES;
using namespace LAMMPS_NS::FixConst;

namespace LAMMPS_NS
{
	FixSSAGES::FixSSAGES(LAMMPS *lmp, int narg, char **arg) : 
	Fix(lmp, narg, arg), Hook()
	{
	}

	void FixSSAGES::setup(int)
	{
		// Allocate vectors for snapshot.
		auto n = atom->natoms + 1;
		auto& pos = _snapshot.GetPositions();
		pos.resize(n);
		auto& vel = _snapshot.GetVelocities();
		vel.resize(n);
		auto& frc = _snapshot.GetForces();
		frc.resize(n);
		auto& ids = _snapshot.GetAtomIDs();
		ids.resize(n);
		auto& types = _snapshot.GetAtomTypes();
		types.resize(n);

		SyncToSnapshot();
		Hook::PreSimulationHook();
	}

	void FixSSAGES::post_force(int)
	{
		SyncToSnapshot();
		Hook::PostIntegration();
	}

	int FixSSAGES::setmask()
	{
	  int mask = 0;
	  mask |= PRE_EXCHANGE;
	  return mask;
	}

	//Will force a call to SyncToEngine?
	void FixSSAGES::SyncToSnapshot() //put LAMMPS values -> Snapshot
	{

		// Obtain local reference to snapshot variables.
		// Const Atom will ensure that atom variables are
		// not being changed. Only snapshot side variables should
		// change.
		const Atom& _atom=atom;

		auto& pos = _snapshot.GetPositions();
		auto& vel = _snapshot.GetVelocities();
		auto& frc = _snapshot.GetForces();

		// Labels and ids for future work on only updating
		// atoms that have changed.
		auto& ids = _snapshot.GetAtomIDs();
		auto& types = _snapshot.GetAtomTypes();

		// Thermo properties
		

		// Positions
		for (int i=0; i<_atom->x.size(); i++)
		{
			pos[i][0]=_atom->x[i][0]; //x
			pos[i][1]=_atom->x[i][1]; //y
			pos[i][2]=_atom->x[i][2]; //z
		}

		// Forces
		for (int i=0; i<_atom->f.size(); i++)
		{
			frc[i][0]=_atom->f[i][0]; //force->x
			frc[i][1]=_atom->f[i][1]; //force->y
			frc[i][2]=_atom->f[i][2]; //force->z
		}

		// Velocities
		for (int i=0; i<_atom->v.size(); i++)
		{
			vel[i][0]=_atom->v[i][0];
			vel[i][1]=_atom->v[i][1];
			vel[i][2]=_atom->v[i][2];
		}

		// IDs
		for (int i=0; i<_atom->tag.size();i++)
		{
			ids[i]=_atom->tag[i];
		}

		// Types
		for (int i=0; i<_atom->type.size();i++)
		{
			types[i]=_atom->type[i];
		}

		// Set thermodynamic information

	}

	void FixSSAGES::SyncToEngine() //put Snapshot values -> LAMMPS
	{
		// Obtain local const reference to snapshot variables.
		// Const will ensure that _snapshot variables are
		// not being changed. Only engine side variables should
		// change. 

		const auto& pos = _snapshot.GetPositions();
		const auto& vel = _snapshot.GetVelocities();
		const auto& frc = _snapshot.GetForces();

		// Labels and ids for future work on only updating
		// atoms that have changed.
		const auto& ids = _snapshot.GetAtomIDs();
		const auto& types = _snapshot.GetAtomTypes();

		// Need to parallize this portion each processor can set and read
		// certain atoms.
		// Loop through all atoms and set their values
		//Positions
		for (int i=0; i<atom->x.size(); i++)
		{
			atom->x[i][0]=pos[i][0]; //x 
			atom->x[i][1]=pos[i][1]; //y
			atom->x[i][2]=pos[i][2]; //z
		}

		// Forces
		for (int i=0; i<atom->f.size(); i++)
		{
			atom->f[i][0]=frc[i][0]; //force->x
			atom->f[i][1]=frc[i][1]; //force->y
			atom->f[i][2]=frc[i][2]; //force->z
		}

		//velocities
		for (int i=0; i<atom->v.size(); i++)
		{
			atom->v[i][0]=vel[i][0]; //velocity->x
			atom->v[i][1]=vel[i][1]; //velocity->y
			atom->v[i][2]=vel[i][2]; //velocity->z
		}

		// IDs
		for (int i=0; i<_atom->tag.size();i++)
		{
			atom->tag[i]=ids[i];
		}

		// Types
		for (int i=0; i<_atom->type.size();i++)
		{
			atom->type[i]=types[i];
		}

		// Set thermodynamic information

	}
}