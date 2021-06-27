//
// Created by shubham on 08.06.21.
//
#include "math.h"
#include "constant.h"
#include <omp.h>
#include <iostream>

namespace OMP{
    // Constant variants
    constexpr double squaredMaxDistance = MAX_DISTANCE * MAX_DISTANCE;

    class OctreeCell {
    private:
        vec3 midpoint;
        double radius;

    public:
        OctreeCell() {
            midpoint = vec3(0, 0, 0);
            radius = 0.;
        }
        OctreeCell(double x, double y, double z, double rad) : midpoint(x, y, z), radius(rad) {}
        OctreeCell(OctreeCell&& o) noexcept : midpoint(std::move(o.midpoint)), radius(std::move(o.radius)) {}
        OctreeCell(const OctreeCell& o) : midpoint(o.midpoint), radius(o.radius) {}
        ~OctreeCell() {}
        OctreeCell& operator=(const OctreeCell& o) {
            if (this != &o) {
                midpoint = vec3(o.midpoint);
                radius = o.radius;
            }
            return *this;
        }
        double getRadius()const {
            return radius;
        }
        vec3 getMidpoint()const {
            return midpoint;
        }
        bool contains(const vec3& p) const {
            bool xcond = p.x <= (midpoint.x + (radius / 2.0)) && p.x >= (midpoint.x - (radius / 2.0));
            bool ycond = p.y <= (midpoint.y + (radius / 2.0)) && p.y >= (midpoint.y - (radius / 2.0));
            bool zcond = p.z <= (midpoint.z + (radius / 2.0)) && p.z >= (midpoint.z - (radius / 2.0));
            return xcond && ycond && zcond;
        }

        OctreeCell UpNorthWest() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x - halfRadius, midpoint.y + halfRadius,
                midpoint.z + halfRadius, 2 * halfRadius);
        }
        OctreeCell UpNorthEast() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x + halfRadius, midpoint.y + halfRadius,
                midpoint.z + halfRadius, 2 * halfRadius);
        }

        OctreeCell UpSouthWest() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x - halfRadius, midpoint.y - halfRadius,
                midpoint.z + halfRadius, 2 * halfRadius);
        }
        OctreeCell UpSouthEast() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x + halfRadius, midpoint.y - halfRadius,
                midpoint.z + halfRadius, 2 * halfRadius);
        }

        OctreeCell DownNorthWest() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x - halfRadius, midpoint.y + halfRadius,
                midpoint.z - halfRadius, 2 * halfRadius);
        }
        OctreeCell DownNorthEast() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x + halfRadius, midpoint.y + halfRadius,
                midpoint.z - halfRadius, 2 * halfRadius);
        }

        OctreeCell DownSouthEast() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x + halfRadius, midpoint.y - halfRadius,
                midpoint.z - halfRadius, 2 * halfRadius);
        }
        OctreeCell DownSouthWest() const {
            double halfRadius = radius / 4.0;
            return OctreeCell(midpoint.x - halfRadius, midpoint.y - halfRadius,
                midpoint.z - halfRadius, 2 * halfRadius);
        }
        bool IsUp(Body& b)const {
            return b.pos.z >= midpoint.z;
        }
        bool IsEast(Body& b)const {
            return b.pos.x >= midpoint.x;
        }
        bool IsNorth(Body& b)const {
            return b.pos.y >= midpoint.y;
        }

    };
    class Octree {
    private:
        Body b;
        OMP::OctreeCell thisNode;
        Octree* upNorthWest;
        Octree* upNorthEast;
        Octree* upSouthWest;
        Octree* upSouthEast;
        Octree* downNorthWest;
        Octree* downNorthEast;
        Octree* downSouthWest;
        Octree* downSouthEast;
        void CaseSelector(Body* b, OMP::OctreeCell& casecell, Octree*** casenode);

    public:
        Octree(OMP::OctreeCell&& o) : thisNode(std::move(o))
        {
            upNorthWest = nullptr;
            upNorthEast = nullptr;
            upSouthWest = nullptr;
            upSouthEast = nullptr;
            downNorthWest = nullptr;
            downNorthEast = nullptr;
            downSouthWest = nullptr;
            downSouthEast = nullptr;
        }
        Octree(const OMP::OctreeCell& o) : thisNode(o)
        {
            upNorthWest = nullptr;
            upNorthEast = nullptr;
            upSouthWest = nullptr;
            upSouthEast = nullptr;
            downNorthWest = nullptr;
            downNorthEast = nullptr;
            downSouthWest = nullptr;
            downSouthEast = nullptr;
        }
        const OMP::OctreeCell& getOctreeCell() const;
        ~Octree();
        bool IsExternal();
        void InsertBody(Body* insertBod);

        void TreeInteract(Body* bod);
    };

    inline double magnitude(double x, double y, double z)
    {
        //Assure we don't call 3 times pow
        //Compiler might optimize it but this way we are sure
        //because 3 multiplications are faster than 3 pows
        return sqrt(x * x + y * y + z * z);
    }
    inline double magnitude(const vec3& v) {
        return magnitude(v.x, v.y, v.z);
    }
    inline double squaredMagnitude(double x, double y, double z)
    {
        return x * x + y * y + z * z;
    }
    inline double squaredMagnitude(const vec3& v) {
        return squaredMagnitude(v.x, v.y, v.z);
    }
    OMP::Octree::~Octree() {
        if (upNorthWest != nullptr) delete upNorthWest;
        if (upNorthEast != nullptr) delete upNorthEast;
        if (upSouthWest != nullptr) delete upSouthWest;
        if (upSouthEast != nullptr) delete upSouthEast;
        if (downNorthWest != nullptr) delete downNorthWest;
        if (downNorthEast != nullptr) delete downNorthEast;
        if (downSouthWest != nullptr) delete downSouthWest;
        if (downSouthEast != nullptr) delete downSouthEast;
    }
    const OMP::OctreeCell& OMP::Octree::getOctreeCell() const {
        return thisNode;
    }
    bool OMP::Octree::IsExternal()
    {
        return (upNorthWest == nullptr) && (upNorthEast == nullptr) && (upSouthWest == nullptr) &&
            (upSouthEast == nullptr) && (downNorthWest == nullptr) && (downNorthEast == nullptr) &&
            (downSouthWest == nullptr) && (downSouthEast == nullptr);
    }
    void OMP::Octree::InsertBody(Body* insertBod)
    {
        if (b.m == 0)
        {
            b = *insertBod;
        }
        else //if (!IsExternal())
        {
            bool isExtern = IsExternal();
            Body* updatedBod;
            if (!isExtern)
            {
                b.pos.x = (insertBod->pos.x * insertBod->m + b.pos.x * b.m) / (insertBod->m + b.m);
                b.pos.y = (insertBod->pos.y * insertBod->m + b.pos.y * b.m) / (insertBod->m + b.m);
                b.pos.z = (insertBod->pos.z * insertBod->m + b.pos.z * b.m) / (insertBod->m + b.m);
                b.m += insertBod->m;
                updatedBod = insertBod;
            }
            else {
                updatedBod = &b;
            }
            OMP::OctreeCell&& casecell = thisNode.DownNorthEast();
            OMP::Octree** casenode;
            CaseSelector(updatedBod, casecell, &casenode);
            if (casecell.contains(updatedBod->pos)) {
                if ((*casenode) == nullptr) {
                    (*casenode) = new OMP::Octree(std::move(casecell));
                }
                (*casenode)->InsertBody(updatedBod);
            }
            if (isExtern) {
                InsertBody(insertBod);
            }
        }
    }
    void OMP::Octree::CaseSelector(Body* body, OMP::OctreeCell& c, OMP::Octree*** casenode) {
        /////////////////////////////////////////////////////////////////////////////////////
        //////// Uses 3 digit binary representation to find the correct octant //////////////
        /////////////////////////////////////////////////////////////////////////////////////

        bool vertUD = thisNode.IsUp(*body);
        bool longNS = thisNode.IsNorth(*body);
        bool latEW = thisNode.IsEast(*body);
        constexpr int UP    = 0b100;
        constexpr int NORTH = 0b010;
        constexpr int EAST  = 0b001;
        constexpr int UNE = UP + NORTH  + EAST;
        constexpr int UNW = UP + NORTH;
        constexpr int USE = UP          + EAST;
        constexpr int USW = UP;
        constexpr int DNE =      NORTH  + EAST;
        constexpr int DNW =      NORTH;
        constexpr int DSE =               EAST;
        constexpr int DSW = 0;
        //    constexpr int DSW = 0;
        const int pos = latEW * EAST + longNS * NORTH + vertUD * UP;
        switch (pos) {
        case UNE: //Up North East
            c = thisNode.UpNorthEast();
            *casenode = &upNorthEast;
            break;
        case UNW: //Up North West
            c = thisNode.UpNorthWest();
            *casenode = &upNorthWest;
            break;
        case USE: //Up South East
            c = thisNode.UpSouthEast();
            *casenode = &upSouthEast;
            break;
        case USW: //Up South West
            c = thisNode.UpSouthWest();
            *casenode = &upSouthWest;
            break;
        case DNE: //Down North East
            c = thisNode.DownNorthEast();
            *casenode = &downNorthEast;
            break;
        case DNW: //Down North West
            c = thisNode.DownNorthWest();
            *casenode = &downNorthWest;
            break;
        case DSE: //Down South East
            c = thisNode.DownSouthEast();
            *casenode = &downSouthEast;
            break;
        default: //Down South West (case 0)
            c = thisNode.DownSouthWest();
            *casenode = &downSouthWest;
            break;
        }
    }

    void BodyInteract(Body& a, Body& b)
    {
        //////////////////////////////////////////////////////
        //////// This is for pairwise interaction ////////////
        //////////////////////////////////////////////////////
        vec3 diff = a.pos - b.pos;
        diff = diff * TO_METERS;

        double dist = magnitude(diff);
        double F = (G * a.m * b.m) / ((dist * dist + SOFTENING * SOFTENING) * dist);

        a.acc = a.acc - diff * (F / a.m);

        b.acc = b.acc + diff * (F / b.m);
    }

    void BodyInteract(Body& target, Body& other, bool singlePart)
    {
        //////////////////////////////////////////////////////////////////
        //////// This is for Tree interaction ////////////////////////////
        //////// Note the difference here only target is updated /////////
        //////////////////////////////////////////////////////////////////
        vec3 diff;
        diff = (target.pos - other.pos) * TO_METERS;
        double dist = OMP::magnitude(diff);

        // this test can be true only when singlePart is true
        // (is always false when singlePart is false = when target is not external)
        if (dist == 0)
            // avoid division by 0
            return;

        double F = (G * target.m * other.m) / ((dist * dist + SOFTENING * SOFTENING) * dist);

        target.acc = target.acc - (diff * F) / target.m;

        //Friction
#if ENABLE_FRICTION
        if (singlePart)
        {
            double friction = 0.5 / pow(2.0, FRICTION_FACTOR * (
                ((dist + SOFTENING)) / (TO_METERS)));
            //	cout << friction << "\n";
            if (friction > 0.0001 && ENABLE_FRICTION)
            {
                target.acc = target.acc + (other.vel - target.vel) * (friction / 2);
            }
        }
#else
        (void)singlePart;
#endif
    }
    void OMP::Octree::TreeInteract(Body* bod)
    {
        //////////////////////////////////////////////////////////////////
        ///////// Computes the force on body due to all the others ///////
        /////////////////////////////////////////////////////////////////
        vec3 diff = b.pos - bod->pos;
        //Compare squares against each other
        //Saves a sqrt (instruction more expensive than a multiplication)
        double squaredDist = OMP::squaredMagnitude(diff);
        double squaredTheta = thisNode.getRadius() * thisNode.getRadius() / squaredDist;
        if (IsExternal())
        {
            // Case when a octree cell contains only one element
            OMP::BodyInteract(*bod, b, true);
        }
        else if (squaredTheta < squaredMaxDistance)
        {
            // Case where octree cell contains more than one element but is taken
            // in aggregated fashion
            OMP::BodyInteract(*bod, b, false);
        }
        else {
            // Case where further refinement is necessary
            if (upNorthWest != NULL) upNorthWest->TreeInteract(bod);
            if (upNorthEast != NULL) upNorthEast->TreeInteract(bod);
            if (upSouthWest != NULL) upSouthWest->TreeInteract(bod);
            if (upSouthEast != NULL) upSouthEast->TreeInteract(bod);
            if (downNorthWest != NULL) downNorthWest->TreeInteract(bod);
            if (downNorthEast != NULL) downNorthEast->TreeInteract(bod);
            if (downSouthWest != NULL) downSouthWest->TreeInteract(bod);
            if (downSouthEast != NULL) downSouthEast->TreeInteract(bod);
        }
    }
    void Integrate(Body* b) {
        ////////////////////////////////////////////////
        ///// Update the velocity and //////////////////
        /// position of the body ///////////////////////
        ////////////////////////////////////////////////
        b->vel = b->vel + (b->acc) * TIME_STEP;
        b->acc = 0.0;
        b->pos = b->pos + (b->vel) * (TIME_STEP / TO_METERS);
    }
    void PositionUpdate(Body* bodies, const int numBodies) {
        for (int i = 0; i < numBodies; i++) {
            OMP::Integrate(&bodies[i]);
        }
    }
    void UpdateStep(Body* bodies, const int numBodies) {
        /////////////////////////////////////////////////////////////
        /// The function sets up tree and update the ////////////////
        /// positions of bodies for one time step ///////////////////
        ////////////////////////////////////////////////////////////

        // Interaction with sun is individual
        Body& sun = bodies[0];
        for (int bcount = 1; bcount < numBodies; bcount++) {
            OMP::BodyInteract(sun, bodies[bcount]);
        }

        OMP::OctreeCell&& root = OMP::OctreeCell(0, //// x center of root
            0,       //// y center of root
            0.1374, //// z center of root
            60 * SYSTEM_SIZE
        );

        OMP::Octree* tree = new OMP::Octree(std::move(root));

        // Now compute the interaction due to foces between the bodies
        // This makes sure that the bodies that are just too far are
        // not included in the computation makes job easier
        for (int bcount = 1; bcount < numBodies; bcount++) {
            // Check if the body lies in the system
            if (tree->getOctreeCell().contains(bodies[bcount].pos)) {
                tree->InsertBody(&bodies[bcount]);
            }
        }
//#pragma omp parallel for 
        for (int bcount = 1; bcount < numBodies; bcount++) {
            // Check if the body lies in the system
            if (tree->getOctreeCell().contains(bodies[bcount].pos)) {
                tree->TreeInteract(&bodies[bcount]);
            }
        }

        // remove the tree
        delete tree;
        OMP::PositionUpdate(bodies, numBodies);
    }

    void Solve(Body* bodies, const int numBodies, const int numSteps);
}

void OMP::Solve(Body *bodies, const int numBodies, const int numSteps) {
    // Implementation here
    if (numSteps < 0) return;
    if (numBodies < 0) return;

    for (size_t step = 1; step < static_cast<size_t>(numSteps); step++) {
        OMP::UpdateStep(bodies, numBodies);
    }
}
