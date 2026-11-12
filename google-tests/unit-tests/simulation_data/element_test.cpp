#include <gtest/gtest.h>

#include <sstream>

#include <simulation_data_export.hpp>

#include "common.hpp"

#include <glm/gtx/io.hpp>

TEST(OpticalProperties, OutputOperator)
{
    std::stringstream ss;

    OpticalPropertySet op;
    op.set_ideal_reflection(OpticalSide::Both);
    ss << op;
    EXPECT_GE(ss.str().length(), 0);
}

TEST(Element, ConstructionSmokeTest)
{
    // Smoke tests
    SimulationData sd;
    auto elem = new_element<SingleElement>(sd);
    auto comp = new_element<CompositeElement>(sd);
    auto st = new_stage(sd, 1);
}

TEST(Element, SingleElementAccessors)
{
    SimulationData sd;
    auto ref_ptr = new_element<SingleElement>(sd);
    SingleElement &ref = *ref_ptr;
    glm::dvec3 zero(0.0, 0.0, 0.0);
    glm::dvec3 khat(0.0, 0.0, 1.0);
    EXPECT_TRUE(is_identical(ref.get_origin_ref(), zero));
    EXPECT_TRUE(is_identical(ref.get_aim_vector_ref(), khat));
    EXPECT_TRUE(is_identical(ref.get_euler_angles(), zero));
    EXPECT_EQ(ref.get_aperture(), nullptr);
    EXPECT_EQ(ref.get_surface(), nullptr);

    EXPECT_TRUE(ref.is_enabled());
    ref.disable();
    EXPECT_FALSE(ref.is_enabled());
    ref.enable();
    EXPECT_TRUE(ref.is_enabled());

    EXPECT_FALSE(ref.is_composite());
    EXPECT_FALSE(ref.is_virtual());
    EXPECT_TRUE(ref.is_single());

    EXPECT_FALSE(ref.is_virtual());
    ref.mark_virtual();
    EXPECT_TRUE(ref.is_virtual());
    ref.unmark_virtual();
    EXPECT_FALSE(ref.is_virtual());

    auto pos = glm::dvec3(2.0, 1.0, -3.0);
    ref.set_origin(pos);
    EXPECT_TRUE(is_identical(ref.get_origin_ref(), pos));

    auto aim = glm::dvec3(-1.0, 0.0, 1.0);
    ref.set_aim_vector(aim);
    EXPECT_TRUE(is_identical(ref.get_aim_vector_ref(), aim));

    // auto eulers = glm::dvec3(0.1, 0.2, -0.3);
    // // ref.set_euler_angles(eulers);
    // EXPECT_TRUE(is_identical(ref.get_euler_angles(), eulers));

    const double ZROT = 0.58;
    ref.set_zrot(ZROT);
    EXPECT_EQ(ref.get_zrot(), ZROT);

    EXPECT_EQ(ref.set_reference_frame_geometry(pos, aim, ZROT), 0);
    EXPECT_FALSE(is_identical(ref.get_euler_angles(), zero));

    const double D = 1.0;
    auto ap = SolTrace::Data::make_aperture<SolTrace::Data::Circle>(D);
    ref.set_aperture(ap);
    auto rap = std::dynamic_pointer_cast<SolTrace::Data::Circle>(ref.get_aperture());
    EXPECT_NE(rap, nullptr);
    EXPECT_EQ(rap->diameter, D);

    const double HA = 0.25;
    auto sp = SolTrace::Data::make_surface<SolTrace::Data::Cone>(HA);
    ref.set_surface(sp);
    auto rsp = std::dynamic_pointer_cast<SolTrace::Data::Cone>(ref.get_surface());
    EXPECT_NE(rsp, nullptr);
    EXPECT_EQ(rsp->half_angle, HA);
}

TEST(Element, VirtualElement)
{
    SimulationData sd;
    auto ve_ptr = new_element<VirtualElement>(sd);
    VirtualElement &ve = *ve_ptr;

    EXPECT_TRUE(ve.is_virtual());

    const double LX = 1.0;
    const double LY = 2.0;
    ve.set_aperture(make_aperture<Rectangle>(LX, LY));
    auto aptr = ve.get_aperture();
    EXPECT_EQ(aptr->get_type(), ApertureType::RECTANGLE);
    auto rptr = std::dynamic_pointer_cast<Rectangle>(aptr);
    EXPECT_NE(rptr, nullptr);
    if (rptr != nullptr)
    {
        EXPECT_EQ(rptr->x_length(), LX);
        EXPECT_EQ(rptr->y_length(), LY);
    }

    ve.set_surface(make_surface<Flat>());
    auto sptr = ve.get_surface();
    EXPECT_EQ(sptr->get_type(), SurfaceType::FLAT);
    auto fptr = std::dynamic_pointer_cast<Flat>(sptr);
    EXPECT_NE(fptr, nullptr);

    EXPECT_TRUE(ve.is_virtual());
    EXPECT_TRUE(ve.is_single());
    EXPECT_FALSE(ve.is_composite());

    return;
}

TEST(Element, VirtualPlane)
{
    const double LX = 5.0;
    const double LY = 2.5;
    SimulationData sd;
    auto vp_ptr = new_element<VirtualPlane>(sd, LX, LY);
    VirtualPlane &vp = *vp_ptr;

    EXPECT_TRUE(vp.is_virtual());
    EXPECT_EQ(vp.get_aperture()->get_type(), ApertureType::RECTANGLE);
    EXPECT_EQ(vp.get_surface()->get_type(), SurfaceType::FLAT);

    auto rptr = std::dynamic_pointer_cast<Rectangle>(vp.get_aperture());
    EXPECT_NE(rptr, nullptr);
    if (rptr != nullptr)
    {
        EXPECT_EQ(rptr->x_length(), LX);
        EXPECT_EQ(rptr->y_length(), LY);
    }

    // These functions should have no effects
    vp.set_aperture(make_aperture<Circle>(2.0));
    EXPECT_EQ(vp.get_aperture()->get_type(), ApertureType::RECTANGLE);
    vp.set_surface(make_surface<Parabola>(1.0, 1.0));
    EXPECT_EQ(vp.get_surface()->get_type(), SurfaceType::FLAT);

    return;
}

TEST(Element, CompositeElementAccessors)
{
    SimulationData sd;
    auto cmp = new_element<CompositeElement>(sd);
    EXPECT_TRUE(cmp->is_composite());

    // Things that should be empty...
    EXPECT_EQ(cmp->get_aperture(), nullptr);
    EXPECT_EQ(cmp->get_surface(), nullptr);
    EXPECT_EQ(cmp->get_optical_property_set_id(), SolTrace::Data::OPTICS_ID_UNASSIGNED);
    EXPECT_EQ(cmp->get_optical_property_set(), nullptr);
    const aperture_ptr ap = cmp->get_aperture();
    EXPECT_EQ(ap, nullptr);
    const surface_ptr sp = cmp->get_surface();
    EXPECT_EQ(sp, nullptr);

    // Add/remove/change elements
    const int NUM_ELEMENTS = 4;
    for (int i = 0; i < NUM_ELEMENTS; ++i)
    {
        auto elem = make_configured_element(sd);
        attach_child(cmp, elem);
        // EXPECT_EQ(elem->get_stage(), STAGE);
    }
    EXPECT_EQ(cmp->get_number_of_elements(), NUM_ELEMENTS);

    cmp->remove_element(0);
    EXPECT_EQ(cmp->get_number_of_elements(), NUM_ELEMENTS - 1);
    EXPECT_EQ(cmp->get_element(0), nullptr);
    auto elem = make_configured_element(sd);
    auto id = attach_child(cmp, elem);
    EXPECT_EQ(cmp->get_element(id).get(), elem.get());

    // Removing an element and adding another one in its place
    EXPECT_EQ(cmp->remove_element(id), 1);
    EXPECT_EQ(cmp->get_element(id), nullptr);
    auto elem2 = make_configured_element(sd);
    EXPECT_NE(elem.get(), elem2.get());
    auto id2 = attach_child(cmp, elem2);
    EXPECT_EQ(cmp->get_element(id2).get(), elem2.get());

    // Recursive CompositeElements -- this is disallowed!
    auto cmp2 = new_element<CompositeElement>(sd);
    auto cid = attach_child(cmp, cmp2);
    EXPECT_FALSE(SolTrace::Data::Element::is_success(cid));
    EXPECT_EQ(cid, SolTrace::Data::ELEMENT_INVALID_SETUP);
    EXPECT_EQ(cmp->get_number_of_elements(), NUM_ELEMENTS);

    elem2->enable();
    cmp->disable();
    EXPECT_FALSE(cmp->is_enabled());
    EXPECT_FALSE(elem2->is_enabled());
    cmp->enable();
    EXPECT_TRUE(cmp->is_enabled());
    EXPECT_TRUE(elem2->is_enabled());

    EXPECT_FALSE(elem2->is_virtual());
    EXPECT_FALSE(cmp->is_virtual());
    cmp->mark_virtual();
    EXPECT_TRUE(cmp->is_virtual());
    EXPECT_TRUE(elem2->is_virtual());
    cmp->unmark_virtual();
    EXPECT_FALSE(cmp->is_virtual());
    EXPECT_FALSE(elem2->is_virtual());

    cmp->mark_virtual();
    auto elem3 = make_configured_element(sd);
    EXPECT_FALSE(elem3->is_virtual());
    attach_child(cmp, elem3);
    EXPECT_TRUE(elem3->is_virtual());
    cmp->unmark_virtual();
    EXPECT_FALSE(elem3->is_virtual());

    // Check that pass through functions are hooked up correctly
    auto iter = cmp->get_iterator();
    while (!cmp->is_at_end(iter))
    {
        ++iter;
    }
    EXPECT_TRUE(cmp->is_at_end(iter));

    auto citer = cmp->get_const_iterator();
    while (!cmp->is_at_end(citer))
    {
        ++citer;
    }
    EXPECT_TRUE(cmp->is_at_end(citer));
}

TEST(Element, StageElementAccessors)
{
    SimulationData sd;
    const int_fast64_t STAGE = 10;
    const int_fast64_t RESET_STAGE = 20;
    auto st1 = new_stage(sd, 10);

    auto el1 = make_configured_element(sd);
    auto cmp1 = new_element<CompositeElement>(sd);
    auto sub1 = make_configured_element(sd);
    auto sub2 = make_configured_element(sd);
    auto sub3 = make_configured_element(sd);
    EXPECT_TRUE(SolTrace::Data::Element::is_success(attach_child(cmp1, sub1)));
    EXPECT_TRUE(SolTrace::Data::Element::is_success(attach_child(cmp1, sub2)));
    EXPECT_TRUE(SolTrace::Data::Element::is_success(attach_child(cmp1, sub3)));

    EXPECT_TRUE(SolTrace::Data::Element::is_success(attach_child(st1, el1)));
    EXPECT_TRUE(SolTrace::Data::Element::is_success(attach_child(st1, cmp1)));

    EXPECT_EQ(st1->get_number_of_elements(), 4);
    EXPECT_EQ(st1->get_stage(), STAGE);
    EXPECT_EQ(el1->get_stage(), STAGE);
    EXPECT_EQ(cmp1->get_stage(), STAGE);
    EXPECT_EQ(sub1->get_stage(), STAGE);

    st1->set_stage(RESET_STAGE);
    EXPECT_EQ(st1->get_stage(), RESET_STAGE);
    EXPECT_EQ(el1->get_stage(), RESET_STAGE);
    EXPECT_EQ(cmp1->get_stage(), RESET_STAGE);
    EXPECT_EQ(sub1->get_stage(), RESET_STAGE);
}

TEST(Element, CoordinateComputationsIdentity)
{
    SimulationData sd;
    auto el = make_configured_element(sd);
    auto st = new_stage(sd, 0);
    glm::dvec3 origin(0.0, 0.0, 0.0);
    glm::dvec3 aim(0.0, 0.0, 1.0);
    glm::dvec3 local(1.0, 1.0, 1.0);
    glm::dvec3 ref(1.0, 1.0, 1.0);
    glm::dvec3 result;
    result = glm::dvec3(0.0);
    double zrot = 0.0;

    el->set_reference_frame_geometry(origin, aim, zrot);
    st->set_reference_frame_geometry(origin, aim, zrot);

    // Conversion test
    el->convert_local_to_global(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);
    el->convert_local_to_stage(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);
    el->convert_local_to_reference(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);

    st->convert_local_to_global(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);
    st->convert_local_to_stage(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);
    st->convert_local_to_reference(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);

    attach_child(st, el);
    el->convert_local_to_global(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);
    el->convert_local_to_stage(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);
    el->convert_local_to_reference(result, local);
    EXPECT_TRUE(is_identical(ref, result));
    result = glm::dvec3(0.0);

    // Origin location tests
    EXPECT_TRUE(is_identical(origin, st->get_origin_ref()));
    EXPECT_TRUE(is_identical(origin, st->get_origin_stage()));
    EXPECT_TRUE(is_identical(origin, st->get_origin_global()));

    EXPECT_TRUE(is_identical(origin, el->get_origin_ref()));
    EXPECT_TRUE(is_identical(origin, el->get_origin_stage()));
    EXPECT_TRUE(is_identical(origin, el->get_origin_global()));

    // Aim vector conversion tests
    EXPECT_TRUE(is_identical(aim, st->get_aim_vector_ref()));
    EXPECT_TRUE(is_identical(aim, st->get_aim_vector_stage()));
    EXPECT_TRUE(is_identical(aim, st->get_aim_vector_global()));

    EXPECT_TRUE(is_identical(aim, el->get_aim_vector_ref()));
    EXPECT_TRUE(is_identical(aim, el->get_aim_vector_stage()));
    EXPECT_TRUE(is_identical(aim, el->get_aim_vector_global()));

    // Operators
    glm::dmat3 Q = glm::identity<glm::dmat3>();

    glm::dmat3 RtoL = el->get_reference_to_local();
    EXPECT_TRUE(is_identical(RtoL, Q));
    glm::dmat3 LtoR = el->get_local_to_reference();
    EXPECT_TRUE(is_identical(LtoR, Q));
    glm::dmat3 StoL = el->get_stage_to_local();
    EXPECT_TRUE(is_identical(StoL, Q));
    glm::dmat3 LtoS = el->get_local_to_stage();
    EXPECT_TRUE(is_identical(LtoS, Q));
    glm::dmat3 GtoL = el->get_global_to_local();
    EXPECT_TRUE(is_identical(GtoL, Q));
    glm::dmat3 LtoG = el->get_local_to_global();
    EXPECT_TRUE(is_identical(LtoG, Q));
}

TEST(Element, CoordinateComputationsRotations)
{
    SimulationData sd;
    using SolTrace::Data::PI;

    // **** Setup Answers **** //
    // Origin
    glm::dvec3 Origin;
    Origin = {};
    // Coordinate transform matrix
    glm::dmat3 Q1;
    Q1[0][0] = 0.5;
    Q1[1][0] = sqrt(3.0) / 2.0;
    Q1[2][0] = 0.0;
    Q1[0][1] = -1.0 / sqrt(2.0);
    Q1[1][1] = 1.0 / sqrt(6.0);
    Q1[2][1] = -1.0 / sqrt(3.0);
    Q1[0][2] = -0.5;
    Q1[1][2] = sqrt(3.0) / 6.0;
    Q1[2][2] = sqrt(6.0) / 3.0;
    //glm::dmat3 Q1t = glm::transpose(Q1);
    // Corresponding Euler angles in radians
    const double a1 = 0.0;
    const double b1 = asin(-1.0 / sqrt(3.0));
    const double g1 = acos(1.0 / cos(b1) * 1.0 / sqrt(6.0)); // approximately 0.615
    // Corresponding aim vector (local z-axis in reference coordinates)
    // glm::dvec3 aim1(0.0, -sqrt(3.0) / 2.0, sqrt(2.0 / 3.0));
    glm::dvec3 aim1(0.0, -1.0 / sqrt(3.0), sqrt(2.0 / 3.0));

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot1 = g1 * 180.0 / PI;

    glm::dmat3 Q2;
    Q2[0][0] = (sqrt(8.0) + sqrt(6.0)) / 8.0;
    Q2[1][0] = (2.0 * sqrt(6.0) - sqrt(2.0)) / 8.0;
    Q2[2][0] = sqrt(6.0) / 4.0;
    Q2[0][1] = -0.75;
    Q2[1][1] = sqrt(3.0) / 4.0;
    Q2[2][1] = 0.5;
    Q2[0][2] = (sqrt(6.0) - sqrt(8.0)) / 8.0;
    Q2[1][2] = (-2.0 * sqrt(6.0) - sqrt(2.0)) / 8.0;
    Q2[2][2] = sqrt(6.0) / 4.0;
    //glm::dmat3 Q2t = glm::transpose(Q2);
    // Corresponding Euler angles in radians
    const double a2 = PI / 4.0;
    const double b2 = PI / 6.0;
    const double g2 = PI / 3.0;
    // Corresponding aim vector (local z-axis in reference coordinates)
    glm::dvec3 aim2(sqrt(3.0 / 8.0), 0.5, sqrt(3.0 / 8.0));

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot2 = 60.0;

    // **** Setup Elements **** //
    auto el = make_configured_element(sd);
    el->set_reference_frame_geometry(Origin, aim1, zrot1);

    auto st = new_stage(sd, 0);
    st->set_reference_frame_geometry(Origin, aim2, zrot2);
    attach_child(st, el);

    // **** Tests **** //
    const double TOL = 1e-12;
    glm::dvec3 scratch;
    glm::dvec3 result_vec;
    glm::dmat3 result_mat;
    // Origin tests
    EXPECT_TRUE(is_identical(el->get_origin_stage(), el->get_origin_ref()));
    EXPECT_TRUE(is_identical(el->get_origin_ref(), Origin));
    EXPECT_TRUE(is_identical(el->get_origin_global(), Origin));

    // Euler angles tests
    result_vec = el->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a1, TOL);
    EXPECT_NEAR(result_vec[1], b1, TOL);
    EXPECT_NEAR(result_vec[2], g1, TOL);
    result_vec = st->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a2, TOL);
    EXPECT_NEAR(result_vec[1], b2, TOL);
    EXPECT_NEAR(result_vec[2], g2, TOL);

    // Aim vector tests
    EXPECT_TRUE(is_identical(el->get_aim_vector_ref(), aim1, TOL));
    EXPECT_TRUE(is_identical(el->get_aim_vector_stage(), el->get_aim_vector_ref(), TOL));
    result_vec = Q2 * aim1;
    EXPECT_TRUE(is_identical(el->get_aim_vector_global(), result_vec, TOL));

    glm::dvec3 v_local(-1.0, 2.0, 4.0);
    glm::dvec3 v_stage;
    glm::dvec3 v_global;
    v_stage = Q1 * v_local;
    v_global = Q2 * v_stage;

    glm::dvec3 result;

    el->convert_local_to_stage(result, v_local);
    EXPECT_TRUE(is_identical(result, v_stage, TOL));
    result = glm::dvec3(0.0);

    el->convert_stage_to_local(result, v_stage);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
    result = glm::dvec3(0.0);

    el->convert_local_to_global(result, v_local);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    el->convert_global_to_local(result, v_global);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
}

TEST(Element, CoordinateComputationsTranslations)
{
    SimulationData sd;
    // **** Setup Answers **** //
    // Origin
    glm::dvec3 Origin1(1.0, 2.0, 3.0);
    // Corresponding Euler angles in radians
    const double a = 0.0;
    const double b = 0.0;
    const double g = 0.0;
    // Corresponding aim vector (local z-axis in reference coordinates)
    // glm::dvec3 aim1(0.0, -sqrt(3.0) / 2.0, sqrt(2.0 / 3.0));
    glm::dvec3 aim1(0.0, 0.0, 1.0);
    aim1 = 1.0 * Origin1 + 1.0 * aim1;

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot = 0.0;

    // Origin
    glm::dvec3 Origin2(-3.0, 1.0, -5.0);
    // Corresponding aim vector (local z-axis in reference coordinates)
    glm::dvec3 aim2(0.0, 0.0, 1.0);
    aim2 = 1.0 * Origin2 + 1.0 * aim2;

    // **** Setup Elements **** //
    auto el = make_configured_element(sd);
    el->set_reference_frame_geometry(Origin1, aim1, zrot);

    auto st = new_stage(sd, 0);
    st->set_reference_frame_geometry(Origin2, aim2, zrot);
    attach_child(st, el);

    // **** Tests **** //
    const double TOL = 1e-12;
    glm::dvec3 scratch;
    glm::dvec3 result_vec;
    glm::dmat3 result_mat;
    // Origin tests
    EXPECT_TRUE(is_identical(el->get_origin_stage(), el->get_origin_ref()));
    EXPECT_TRUE(is_identical(el->get_origin_ref(), Origin1));
    result_vec = 1.0 * Origin1 + 1.0 * Origin2;
    EXPECT_TRUE(is_identical(el->get_origin_global(), result_vec));

    // Euler angles tests
    result_vec = el->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a, TOL);
    EXPECT_NEAR(result_vec[1], b, TOL);
    EXPECT_NEAR(result_vec[2], g, TOL);
    result_vec = st->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a, TOL);
    EXPECT_NEAR(result_vec[1], b, TOL);
    EXPECT_NEAR(result_vec[2], g, TOL);

    // Aim vector tests
    EXPECT_TRUE(is_identical(el->get_aim_vector_ref(), aim1, TOL));
    EXPECT_TRUE(is_identical(el->get_aim_vector_stage(), el->get_aim_vector_ref(), TOL));
    result_vec = 1.0 * Origin2 + 1.0 * aim1;
    EXPECT_TRUE(is_identical(el->get_aim_vector_global(), result_vec, TOL));

    glm::dvec3 v_local(-1.0, 2.0, 4.0);
    glm::dvec3 v_stage;
    glm::dvec3 v_global;
    v_stage = 1.0 * Origin1 + 1.0 * v_local;
    v_global = 1.0 * Origin2 + 1.0 * v_stage;

    glm::dvec3 result;

    el->convert_local_to_stage(result, v_local);
    EXPECT_TRUE(is_identical(result, v_stage, TOL));
    result = glm::dvec3(0.0);

    el->convert_stage_to_local(result, v_stage);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
    result = glm::dvec3(0.0);

    el->convert_local_to_global(result, v_local);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    el->convert_global_to_local(result, v_global);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
}

TEST(Element, CoordinateComputations)
{
    SimulationData sd;
    using SolTrace::Data::MatrixTranspose;
    using SolTrace::Data::PI;

    // **** Setup Answers **** //
    // Origin
    glm::dvec3 Origin1(1.0, 2.0, 3.0);
    // Coordinate transform matrix -- stage to local
    glm::dmat3 Q1;
    Q1[0][0] = 0.5;
    Q1[1][0] = sqrt(3.0) / 2.0;
    Q1[2][0] = 0.0;
    Q1[0][1] = -1.0 / sqrt(2.0);
    Q1[1][1] = 1.0 / sqrt(6.0);
    Q1[2][1] = -1.0 / sqrt(3.0);
    Q1[0][2] = -0.5;
    Q1[1][2] = sqrt(3.0) / 6.0;
    Q1[2][2] = sqrt(6.0) / 3.0;
    // Local to stage matrix
    //glm::dmat3 Q1t = glm::transpose(Q1);
    // Corresponding Euler angles in radians
    const double a1 = 0.0;
    const double b1 = asin(-1.0 / sqrt(3.0));
    const double g1 = acos(1.0 / cos(b1) * 1.0 / sqrt(6.0)); // approximately 0.615
    // Corresponding aim vector (local z-axis in reference coordinates)
    // glm::dvec3 aim1(0.0, -sqrt(3.0) / 2.0, sqrt(2.0 / 3.0));
    glm::dvec3 aim1(0.0, -1.0 / sqrt(3.0), sqrt(2.0 / 3.0));
    aim1 = 1.0 * Origin1 + 1.0 * aim1;

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot1 = g1 * 180.0 / PI;

    // Origin
    glm::dvec3 Origin2(-3.0, 1.0, -5.0);
    // Global to stage matrix
    glm::dmat3 Q2;
    Q2[0][0] = (sqrt(8.0) + sqrt(6.0)) / 8.0;
    Q2[1][0] = (2.0 * sqrt(6.0) - sqrt(2.0)) / 8.0;
    Q2[2][0] = sqrt(6.0) / 4.0;
    Q2[0][1] = -0.75;
    Q2[1][1] = sqrt(3.0) / 4.0;
    Q2[2][1] = 0.5;
    Q2[0][2] = (sqrt(6.0) - sqrt(8.0)) / 8.0;
    Q2[1][2] = (-2.0 * sqrt(6.0) - sqrt(2.0)) / 8.0;
    Q2[2][2] = sqrt(6.0) / 4.0;
    // Stage to global matrix
    //glm::dmat3 Q2t = glm::transpose(Q2);
    ;
    // Corresponding Euler angles in radians
    const double a2 = PI / 4.0;
    const double b2 = PI / 6.0;
    const double g2 = PI / 3.0;
    // Corresponding aim vector (local z-axis in reference coordinates)
    glm::dvec3 aim2(sqrt(3.0 / 8.0), 0.5, sqrt(3.0 / 8.0));
    aim2 = 1.0 * Origin2 + 1.0 * aim2;

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot2 = 60.0;

    // **** Setup Elements **** //
    auto el = make_configured_element(sd);
    el->set_reference_frame_geometry(Origin1, aim1, zrot1);

    auto st = new_stage(sd, 0);
    st->set_reference_frame_geometry(Origin2, aim2, zrot2);
    attach_child(st, el);

    // **** Tests **** //
    const double TOL = 1e-12;
    glm::dvec3 scratch;
    glm::dvec3 result_vec;
    glm::dmat3 result_mat;
    // Origin tests
    EXPECT_TRUE(is_identical(el->get_origin_stage(), el->get_origin_ref()));
    EXPECT_TRUE(is_identical(el->get_origin_ref(), Origin1));
    // vector_add(1.0, Origin1, 1.0, Origin2, result_vec);
    result_vec = Q2 * Origin1;
    result_vec = Origin2 + result_vec;
    EXPECT_TRUE(is_identical(el->get_origin_global(), result_vec, TOL));

    // Euler angles tests
    result_vec = el->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a1, TOL);
    EXPECT_NEAR(result_vec[1], b1, TOL);
    EXPECT_NEAR(result_vec[2], g1, TOL);
    result_vec = st->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a2, TOL);
    EXPECT_NEAR(result_vec[1], b2, TOL);
    EXPECT_NEAR(result_vec[2], g2, TOL);

    // Aim vector tests
    EXPECT_TRUE(is_identical(el->get_aim_vector_ref(), aim1, TOL));
    EXPECT_TRUE(is_identical(el->get_aim_vector_stage(), el->get_aim_vector_ref(), TOL));
    result_vec = Q2 * aim1;
    result_vec = Origin2 + result_vec;
    EXPECT_TRUE(is_identical(el->get_aim_vector_global(), result_vec, TOL));

    glm::dvec3 v_local(-1.0, 2.0, 4.0);
    glm::dvec3 v_stage;
    glm::dvec3 v_global;
    v_stage = Q1 * v_local;
    v_stage = Origin1 + v_stage;
    v_global = Q2 * v_stage;
    v_global = Origin2 + v_global;

    glm::dvec3 result;

    el->convert_local_to_stage(result, v_local);
    EXPECT_TRUE(is_identical(result, v_stage, TOL));
    result = glm::dvec3(0.0);

    el->convert_stage_to_local(result, v_stage);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
    result = glm::dvec3(0.0);

    el->convert_local_to_global(result, v_local);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    el->convert_global_to_local(result, v_global);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
    result = glm::dvec3(0.0);

    // Note: Global to reference and reference to global
    // use other conversion routines so just need to test
    // that the if-else statement is correct.

    // Stage coordinates are the reference coordinates
    el->convert_global_to_reference(result, v_global);
    EXPECT_TRUE(is_identical(result, v_stage, TOL));
    result = glm::dvec3(0.0);

    el->convert_reference_to_global(result, v_stage);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    // Global coordinates are reference coordinates
    st->convert_global_to_reference(result, v_global);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    st->convert_reference_to_global(result, v_global);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);
}

TEST(Element, VectorCoordinateComputations)
{
    SimulationData sd;
    using SolTrace::Data::MatrixTranspose;
    using SolTrace::Data::PI;

    // **** Setup Answers **** //
    // Origin
    glm::dvec3 Origin1(1.0, 2.0, 3.0);
    // Coordinate transform matrix -- stage to local
    glm::dmat3 Q1;
    Q1[0][0] = 0.5;
    Q1[1][0] = sqrt(3.0) / 2.0;
    Q1[2][0] = 0.0;
    Q1[0][1] = -1.0 / sqrt(2.0);
    Q1[1][1] = 1.0 / sqrt(6.0);
    Q1[2][1] = -1.0 / sqrt(3.0);
    Q1[0][2] = -0.5;
    Q1[1][2] = sqrt(3.0) / 6.0;
    Q1[2][2] = sqrt(6.0) / 3.0;
    // Local to stage matrix
    //glm::dmat3 Q1t = glm::transpose(Q1);
    // Corresponding Euler angles in radians
    const double a1 = 0.0;
    const double b1 = asin(-1.0 / sqrt(3.0));
    const double g1 = acos(1.0 / cos(b1) * 1.0 / sqrt(6.0)); // approximately 0.615
    // Corresponding aim vector (local z-axis in reference coordinates)
    // glm::dvec3 aim1(0.0, -sqrt(3.0) / 2.0, sqrt(2.0 / 3.0));
    glm::dvec3 aim1(0.0, -1.0 / sqrt(3.0), sqrt(2.0 / 3.0));
    aim1 = Origin1 + aim1;

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot1 = g1 * 180.0 / PI;

    // Origin
    glm::dvec3 Origin2(-3.0, 1.0, -5.0);
    // Global to stage matrix
    glm::dmat3 Q2;
    Q2[0][0] = (sqrt(8.0) + sqrt(6.0)) / 8.0;
    Q2[1][0] = (2.0 * sqrt(6.0) - sqrt(2.0)) / 8.0;
    Q2[2][0] = sqrt(6.0) / 4.0;
    Q2[0][1] = -0.75;
    Q2[1][1] = sqrt(3.0) / 4.0;
    Q2[2][1] = 0.5;
    Q2[0][2] = (sqrt(6.0) - sqrt(8.0)) / 8.0;
    Q2[1][2] = (-2.0 * sqrt(6.0) - sqrt(2.0)) / 8.0;
    Q2[2][2] = sqrt(6.0) / 4.0;
    // Stage to global matrix
    //glm::dmat3 Q2t = glm::transpose(Q2);
    // Corresponding Euler angles in radians
    const double a2 = PI / 4.0;
    const double b2 = PI / 6.0;
    const double g2 = PI / 3.0;
    // Corresponding aim vector (local z-axis in reference coordinates)
    glm::dvec3 aim2(sqrt(3.0 / 8.0), 0.5, sqrt(3.0 / 8.0));
    aim2 = Origin2 + aim2;

    // Z-Rotation is the last of the Euler angles but in degrees
    const double zrot2 = 60.0;

    // **** Setup Elements **** //
    auto el = make_configured_element(sd);
    el->set_reference_frame_geometry(Origin1, aim1, zrot1);

    auto st = new_stage(sd, 0);
    st->set_reference_frame_geometry(Origin2, aim2, zrot2);
    attach_child(st, el);

    // **** Tests **** //
    const double TOL = 1e-12;
    glm::dvec3 scratch;
    glm::dvec3 result_vec;
    glm::dmat3 result_mat;

    // Euler angles tests
    result_vec = el->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a1, TOL);
    EXPECT_NEAR(result_vec[1], b1, TOL);
    EXPECT_NEAR(result_vec[2], g1, TOL);
    result_vec = st->get_euler_angles();
    EXPECT_NEAR(result_vec[0], a2, TOL);
    EXPECT_NEAR(result_vec[1], b2, TOL);
    EXPECT_NEAR(result_vec[2], g2, TOL);

    glm::dvec3 v_local(-1.0, 2.0, 4.0);
    glm::dvec3 v_stage;
    glm::dvec3 v_global;
    v_stage = Q1 * v_local;
    v_global = Q2 * v_stage;

    glm::dvec3 result;

    el->convert_vector_local_to_stage(result, v_local);
    EXPECT_TRUE(is_identical(result, v_stage, TOL));
    result = glm::dvec3(0.0);

    el->convert_vector_stage_to_local(result, v_stage);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
    result = glm::dvec3(0.0);

    el->convert_vector_local_to_global(result, v_local);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    el->convert_vector_global_to_local(result, v_global);
    EXPECT_TRUE(is_identical(result, v_local, TOL));
    result = glm::dvec3(0.0);

    // Note: Global to reference and reference to global
    // use other conversion routines so just need to test
    // that the if-else statement is correct.

    // Stage coordinates are the reference coordinates
    el->convert_vector_global_to_reference(result, v_global);
    EXPECT_TRUE(is_identical(result, v_stage, TOL));
    result = glm::dvec3(0.0);

    el->convert_vector_reference_to_global(result, v_stage);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    // Global coordinates are reference coordinates
    st->convert_vector_global_to_reference(result, v_global);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);

    st->convert_vector_reference_to_global(result, v_global);
    EXPECT_TRUE(is_identical(result, v_global, TOL));
    result = glm::dvec3(0.0);
}

TEST(Element, SingleElementEnforceUserFieldsSet)
{
    SimulationData sd;
    OpticalPropertySet opt_set(InteractionType::REFLECTION, 1, 1);
    opt_set.set_properties(OpticalSide::Both, DistributionType::GAUSSIAN,
        0.75, 0.25, 0.1, 0.001);
    auto optics_ref = sd.add_optical_property_set(opt_set);

    auto elem = new_element<SingleElement>(sd);
    elem->set_optical_property_set(optics_ref);

    // SingleElement requires aperture and surface to be set
    // Test that it throws when aperture is missing
    EXPECT_THROW(elem->enforce_user_fields_set(), std::invalid_argument);

    // Set aperture but not surface - should still throw
    elem->set_aperture(SolTrace::Data::make_aperture<SolTrace::Data::Circle>(1.0));
    EXPECT_THROW(elem->enforce_user_fields_set(), std::invalid_argument);

    // Set both aperture and surface - should not throw
    elem->set_surface(SolTrace::Data::make_surface<SolTrace::Data::Flat>());
    EXPECT_NO_THROW(elem->enforce_user_fields_set());
}

TEST(Element, CompositeElementEnforceUserFieldsSet)
{
    SimulationData sd;
    auto comp = new_element<CompositeElement>(sd);

    // CompositeElement requires at least one subelement
    // Test that it throws when no subelements are present
    EXPECT_THROW(comp->enforce_user_fields_set(), std::invalid_argument);

    // An unconfigured child can be attached, since the child is configured
    // after it is created, but the composite is not valid until it is
    auto elem1 = new_element<SingleElement>(sd);
    auto key1 = attach_child(comp, elem1);
    EXPECT_TRUE(SolTrace::Data::Element::is_success(key1));
    EXPECT_THROW(comp->enforce_user_fields_set(), std::invalid_argument);
    EXPECT_EQ(comp->remove_element(key1), 1);

    // Add properly configured child elements
    auto elem2 = make_configured_element(sd);
    set_element_shape(sd, elem2,
        SolTrace::Data::make_aperture<SolTrace::Data::Rectangle>(2.0, 3.0),
        SolTrace::Data::make_surface<SolTrace::Data::Parabola>(1.0, 2.0));
    EXPECT_NO_THROW(attach_child(comp, elem2));

    // Should still not throw for the CompositeElement
    EXPECT_NO_THROW(comp->enforce_user_fields_set());
}
