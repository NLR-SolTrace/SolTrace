#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

#include <simulation_data_export.hpp>
#include <surface.hpp>

TEST(Surface, Typing)
{
    // Test that each constructor properly sets the type
    auto cone = SolTrace::Data::make_surface<SolTrace::Data::Cone>(50.0);
    EXPECT_EQ(cone->get_type(), SolTrace::Data::CONE);

    auto cylinder = SolTrace::Data::make_surface<SolTrace::Data::Cylinder>(1.0);
    EXPECT_EQ(cylinder->get_type(), SolTrace::Data::CYLINDER);

    auto flat = SolTrace::Data::make_surface<SolTrace::Data::Flat>();
    EXPECT_EQ(flat->get_type(), SolTrace::Data::FLAT);

    auto para = SolTrace::Data::make_surface<SolTrace::Data::Parabola>(1.0, 1.0);
    EXPECT_EQ(para->get_type(), SolTrace::Data::PARABOLA);

    auto sph = SolTrace::Data::make_surface<SolTrace::Data::Sphere>(10.0);
    EXPECT_EQ(sph->get_type(), SolTrace::Data::SPHERE);
}

TEST(Surface, MakeSurfaceFromType)
{
    // Test CONE creation with valid arguments
    {
        std::vector<double> args = {45.0};
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::CONE, args);
        ASSERT_NE(surface, nullptr);
        EXPECT_EQ(surface->get_type(), SolTrace::Data::CONE);
        
        auto cone = std::dynamic_pointer_cast<SolTrace::Data::Cone>(surface);
        ASSERT_NE(cone, nullptr);
        EXPECT_DOUBLE_EQ(cone->half_angle, 45.0);
    }

    // Test CONE creation with insufficient arguments
    {
        std::vector<double> args; // Empty args
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::CONE, args);
        EXPECT_EQ(surface, nullptr);
    }

    // Test CYLINDER creation with valid arguments (note: args[0] is inverted to get radius)
    {
        std::vector<double> args = {0.5}; // 1/0.5 = 2.0 radius
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::CYLINDER, args);
        ASSERT_NE(surface, nullptr);
        EXPECT_EQ(surface->get_type(), SolTrace::Data::CYLINDER);
        
        auto cylinder = std::dynamic_pointer_cast<SolTrace::Data::Cylinder>(surface);
        ASSERT_NE(cylinder, nullptr);
        EXPECT_DOUBLE_EQ(cylinder->radius, 2.0);
    }

    // Test CYLINDER creation with insufficient arguments
    {
        std::vector<double> args; // Empty args
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::CYLINDER, args);
        EXPECT_EQ(surface, nullptr);
    }

    // Test FLAT creation (no parameters needed)
    {
        std::vector<double> args; // Empty args for flat surface
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::FLAT, args);
        ASSERT_NE(surface, nullptr);
        EXPECT_EQ(surface->get_type(), SolTrace::Data::FLAT);
    }

    // Test PARABOLA creation with valid arguments (requires 2 parameters)
    {
        std::vector<double> args = {1.5, 2.0};
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::PARABOLA, args);
        ASSERT_NE(surface, nullptr);
        EXPECT_EQ(surface->get_type(), SolTrace::Data::PARABOLA);
        
        auto parabola = std::dynamic_pointer_cast<SolTrace::Data::Parabola>(surface);
        ASSERT_NE(parabola, nullptr);
        EXPECT_DOUBLE_EQ(parabola->vertex_curv_x, 1.5);
        EXPECT_DOUBLE_EQ(parabola->vertex_curv_y, 2.0);
    }

    // Test PARABOLA creation with insufficient arguments
    {
        std::vector<double> args = {1.5}; // Only 1 argument, needs 2
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::PARABOLA, args);
        EXPECT_EQ(surface, nullptr);
        
        std::vector<double> empty_args; // No arguments
        auto surface2 = SolTrace::Data::make_surface_from_type(SolTrace::Data::PARABOLA, empty_args);
        EXPECT_EQ(surface2, nullptr);
    }

    // Test SPHERE creation with valid arguments
    {
        std::vector<double> args = {0.1}; // vertex curvature
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::SPHERE, args);
        ASSERT_NE(surface, nullptr);
        EXPECT_EQ(surface->get_type(), SolTrace::Data::SPHERE);
        
        auto sphere = std::dynamic_pointer_cast<SolTrace::Data::Sphere>(surface);
        ASSERT_NE(sphere, nullptr);
        EXPECT_DOUBLE_EQ(sphere->vertex_curv, 0.1);
    }

    // Test SPHERE creation with insufficient arguments
    {
        std::vector<double> args; // Empty args
        auto surface = SolTrace::Data::make_surface_from_type(SolTrace::Data::SPHERE, args);
        EXPECT_EQ(surface, nullptr);
    }

    // Test unimplemented surface types (should return nullptr)
    {
        std::vector<double> args = {1.0, 2.0};
        
        auto hyper = SolTrace::Data::make_surface_from_type(SolTrace::Data::HYPER, args);
        EXPECT_EQ(hyper, nullptr);
        
        auto spencer_murty = SolTrace::Data::make_surface_from_type(SolTrace::Data::GENERAL_SPENCER_MURTY, args);
        EXPECT_EQ(spencer_murty, nullptr);
        
        auto torus = SolTrace::Data::make_surface_from_type(SolTrace::Data::TORUS, args);
        EXPECT_EQ(torus, nullptr);
        
        auto unknown = SolTrace::Data::make_surface_from_type(SolTrace::Data::SURFACE_UNKNOWN, args);
        EXPECT_EQ(unknown, nullptr);
    }

    // Test with various argument vectors to ensure robustness
    {
        // Test that extra arguments don't cause issues (should be ignored)
        std::vector<double> multi_args = {5.0, 7.5, 10.0, 12.5}; // Extra args should be ignored
        
        // CONE only uses first argument
        auto cone = SolTrace::Data::make_surface_from_type(SolTrace::Data::CONE, multi_args);
        ASSERT_NE(cone, nullptr);
        EXPECT_EQ(cone->get_type(), SolTrace::Data::CONE);
        auto cone_cast = std::dynamic_pointer_cast<SolTrace::Data::Cone>(cone);
        EXPECT_DOUBLE_EQ(cone_cast->half_angle, 5.0);
        
        // CYLINDER only uses first argument
        auto cylinder = SolTrace::Data::make_surface_from_type(SolTrace::Data::CYLINDER, multi_args);
        ASSERT_NE(cylinder, nullptr);
        EXPECT_EQ(cylinder->get_type(), SolTrace::Data::CYLINDER);
        auto cylinder_cast = std::dynamic_pointer_cast<SolTrace::Data::Cylinder>(cylinder);
        EXPECT_DOUBLE_EQ(cylinder_cast->radius, 1.0/5.0);
        
        // PARABOLA uses first two arguments
        auto parabola = SolTrace::Data::make_surface_from_type(SolTrace::Data::PARABOLA, multi_args);
        ASSERT_NE(parabola, nullptr);
        EXPECT_EQ(parabola->get_type(), SolTrace::Data::PARABOLA);
        auto para_cast = std::dynamic_pointer_cast<SolTrace::Data::Parabola>(parabola);
        EXPECT_DOUBLE_EQ(para_cast->vertex_curv_x, 5.0);
        EXPECT_DOUBLE_EQ(para_cast->vertex_curv_y, 7.5);
        
        // SPHERE only uses first argument
        auto sphere = SolTrace::Data::make_surface_from_type(SolTrace::Data::SPHERE, multi_args);
        ASSERT_NE(sphere, nullptr);
        EXPECT_EQ(sphere->get_type(), SolTrace::Data::SPHERE);
        auto sphere_cast = std::dynamic_pointer_cast<SolTrace::Data::Sphere>(sphere);
        EXPECT_DOUBLE_EQ(sphere_cast->vertex_curv, 5.0);
    }
}

TEST(Cone, MakeCopy)
{
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    double z0, z1;
    double z0copy, z1copy;

    auto cone = make_surface<Cone>(35.0);
    auto copy = cone->make_copy();

    cone->bounding_box(xbox, ybox, z0, z1);
    copy->bounding_box(xbox, ybox, z0copy, z1copy);
    EXPECT_EQ(z0, z0copy);
    EXPECT_EQ(z1, z1copy);
    EXPECT_EQ(cone->z(xbox[0], ybox[1]), copy->z(xbox[0], ybox[1]));
}

TEST(Cone, BoundingBox)
{
    const double TOL = 1e-12;
    const double THETA = 30.0;
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    double z0, z1;

    auto cone = make_surface<Cone>(THETA);
    cone->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, 0.0, TOL);
    EXPECT_NEAR(z1, sqrt(4.0 + 1.0) / tan(THETA * D2R), TOL);
}

TEST(Cylinder, MakeCopy)
{
    const double xbox[2] = {-1.0, 1.0};
    const double ybox[2] = {0.0, 5.0};
    double z0, z1;
    double z0copy, z1copy;

    auto cylin = make_surface<Cylinder>(1.0);
    auto copy = cylin->make_copy();

    cylin->bounding_box(xbox, ybox, z0, z1);
    copy->bounding_box(xbox, ybox, z0copy, z1copy);
    EXPECT_EQ(z0, z0copy);
    EXPECT_EQ(z1, z1copy);
    EXPECT_EQ(cylin->z(xbox[0], ybox[1]), copy->z(xbox[0], ybox[1]));
}

TEST(Cylinder, BoundingBox)
{
    const double TOL = 1e-12;
    const double R = 1.5;
    const double xbox[2] = {-R, R};
    const double ybox[2] = {0.0, 5.0};
    double z0, z1;

    auto cylin = make_surface<Cylinder>(R);
    auto copy = cylin->make_copy();

    cylin->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, -R, TOL);
    EXPECT_NEAR(z1, R, TOL);
}

TEST(Flat, MakeCopy)
{
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    double z0, z1;
    double z0copy, z1copy;

    auto flat = make_surface<Flat>();
    auto copy = flat->make_copy();

    flat->bounding_box(xbox, ybox, z0, z1);
    copy->bounding_box(xbox, ybox, z0copy, z1copy);
    EXPECT_EQ(z0, z0copy);
    EXPECT_EQ(z1, z1copy);
    EXPECT_EQ(flat->z(xbox[0], ybox[1]), copy->z(xbox[0], ybox[1]));
}

TEST(Flat, BoundingBox)
{
    const double TOL = 1e-12;
    const double xbox[2] = {-PI, 2.0 * PI};
    const double ybox[2] = {0.0, 5.0};
    double z0, z1;

    auto flat = make_surface<Flat>();
    auto copy = flat->make_copy();

    flat->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, -1e-4, TOL);
    EXPECT_NEAR(z1, 1e-4, TOL);
}

TEST(Parabola, MakeCopy)
{
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    double z0, z1;
    double z0copy, z1copy;

    auto para = make_surface<Parabola>(1.0, 2.0);
    auto copy = para->make_copy();

    para->bounding_box(xbox, ybox, z0, z1);
    copy->bounding_box(xbox, ybox, z0copy, z1copy);
    EXPECT_EQ(z0, z0copy);
    EXPECT_EQ(z1, z1copy);
    EXPECT_EQ(para->z(xbox[0], ybox[1]), copy->z(xbox[0], ybox[1]));
}

TEST(Parabola, BoundingBox)
{
    const double TOL = 1e-12;
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    const double CX = 0.5;
    const double CY = 0.25;
    double z0, z1;

    auto para = make_surface<Parabola>(CX, CY);

    para->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, 0.0, TOL);
    EXPECT_NEAR(z1, 1.125, TOL);

    const double xbox2[2] = {1.0, 4.0};
    const double ybox2[2] = {0.5, 1.0};

    para->bounding_box(xbox2, ybox2, z0, z1);
    EXPECT_NEAR(z0, 0.5 * (CX * 1.0 + CY * 0.25), TOL);
    EXPECT_NEAR(z1, 0.5 * (CX * 16.0 + CY * 1.0), TOL);
}

TEST(Parabola, BoundingBoxNegativeCurvature)
{
    const double TOL = 1e-12;
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    double z0, z1;

    auto para = make_surface<Parabola>(-0.5, -0.25);

    para->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, -1.125, TOL);
    EXPECT_NEAR(z1, 0.0, TOL);
}

TEST(Parabola, BoundingBoxMixedSignCurvature)
{
    const double TOL = 1e-12;
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {-2.0, 1.0};
    double z0, z1;

    // Saddle: z = 0.5 * (0.5 x^2 - 0.25 y^2)
    auto para = make_surface<Parabola>(0.5, -0.25);

    para->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, -0.5, TOL);
    EXPECT_NEAR(z1, 1.0, TOL);
}

TEST(Parabola, ZValue)
{
    const double TOL = 1e-12;

    // f = 1 => c = 0.5; z = x^2 / (4 f)
    auto para = make_surface<Parabola>(0.5, 0.5);
    EXPECT_NEAR(para->z(2.0, 0.0), 1.0, TOL);
    EXPECT_NEAR(para->z(0.0, 2.0), 1.0, TOL);
    EXPECT_NEAR(para->z(2.0, 2.0), 2.0, TOL);

    // Flat in y
    auto trough = make_surface<Parabola>(0.5, 0.0);
    EXPECT_NEAR(trough->z(2.0, 5.0), 1.0, TOL);
}

TEST(Parabola, CurvatureFromFocalLength)
{
    const double NAN_VAL = std::numeric_limits<double>::quiet_NaN();
    const double INF_VAL = std::numeric_limits<double>::infinity();

    EXPECT_DOUBLE_EQ(Parabola::curvature_from_focal_length(1.0), 0.5);
    EXPECT_DOUBLE_EQ(Parabola::curvature_from_focal_length(-2.0), -0.25);
    EXPECT_DOUBLE_EQ(Parabola::curvature_from_focal_length(INF_VAL), 0.0);
    EXPECT_DOUBLE_EQ(Parabola::curvature_from_focal_length(-INF_VAL), 0.0);

    EXPECT_THROW(Parabola::curvature_from_focal_length(0.0), std::invalid_argument);
    EXPECT_THROW(Parabola::curvature_from_focal_length(NAN_VAL), std::invalid_argument);
}

TEST(Parabola, JsonRoundTripFlatInY)
{
    auto para = make_surface<Parabola>(0.25, 0.0);

    nlohmann::ordered_json jnode;
    para->write_json(jnode);
    EXPECT_EQ(jnode.at("vertex_curv_x").get<double>(), 0.25);
    EXPECT_EQ(jnode.at("vertex_curv_y").get<double>(), 0.0);

    // Dump and parse to mimic file I/O
    auto jparsed = nlohmann::ordered_json::parse(jnode.dump());
    auto read_ptr = SolTrace::Data::make_surface_from_json(jparsed);
    auto read = std::dynamic_pointer_cast<Parabola>(read_ptr);
    ASSERT_NE(read, nullptr);
    EXPECT_DOUBLE_EQ(read->vertex_curv_x, 0.25);
    EXPECT_DOUBLE_EQ(read->vertex_curv_y, 0.0);
}

TEST(Sphere, MakeCopy)
{
    const double xbox[2] = {-1.0, 2.0};
    const double ybox[2] = {0.0, 1.0};
    double z0, z1;
    double z0copy, z1copy;

    auto sph = make_surface<Sphere>(0.5);
    auto copy = sph->make_copy();

    sph->bounding_box(xbox, ybox, z0, z1);
    copy->bounding_box(xbox, ybox, z0copy, z1copy);
    EXPECT_EQ(z0, z0copy);
    EXPECT_EQ(z1, z1copy);
    EXPECT_EQ(sph->z(xbox[0], ybox[1]), copy->z(xbox[0], ybox[1]));
}

TEST(Sphere, BoundingBox)
{
    const double TOL = 1e-12;
    const double CURV = 0.25;
    const double R = 1.0 / CURV;
    const double xbox[2] = {-1.0, 4.0};
    const double ybox[2] = {0.0, 3.0};
    double z0, z1;

    auto sph = make_surface<Sphere>(CURV);

    sph->bounding_box(xbox, ybox, z0, z1);
    EXPECT_NEAR(z0, 0.0, TOL);
    EXPECT_NEAR(z1, R, TOL);

    const double xbox2[2] = {0.0, 1.0};
    const double ybox2[2] = {0.0, 1.0};
    const double rsq = 2.0;

    sph->bounding_box(xbox2, ybox2, z0, z1);
    EXPECT_NEAR(z0, 0.0, TOL);
    EXPECT_NEAR(z1, R - sqrt(R*R - rsq), TOL);
}

TEST(Cone, Validate)
{
    const double NAN_VAL = std::numeric_limits<double>::quiet_NaN();
    const double INF_VAL = std::numeric_limits<double>::infinity();

    EXPECT_NO_THROW(make_surface<Cone>(1.0));
    EXPECT_NO_THROW(make_surface<Cone>(45.0));
    EXPECT_NO_THROW(make_surface<Cone>(89.9));

    EXPECT_THROW(make_surface<Cone>(0.0),    std::invalid_argument);
    EXPECT_THROW(make_surface<Cone>(-1.0),   std::invalid_argument);
    EXPECT_THROW(make_surface<Cone>(90.0),   std::invalid_argument);
    EXPECT_THROW(make_surface<Cone>(91.0),   std::invalid_argument);
    EXPECT_THROW(make_surface<Cone>(NAN_VAL),  std::invalid_argument);
    EXPECT_THROW(make_surface<Cone>(INF_VAL),  std::invalid_argument);
    EXPECT_THROW(make_surface<Cone>(-INF_VAL), std::invalid_argument);
}

TEST(Cylinder, Validate)
{
    const double NAN_VAL = std::numeric_limits<double>::quiet_NaN();
    const double INF_VAL = std::numeric_limits<double>::infinity();

    EXPECT_NO_THROW(make_surface<Cylinder>(0.001));
    EXPECT_NO_THROW(make_surface<Cylinder>(1.0));

    EXPECT_THROW(make_surface<Cylinder>(0.0),    std::invalid_argument);
    EXPECT_THROW(make_surface<Cylinder>(-1.0),   std::invalid_argument);
    EXPECT_THROW(make_surface<Cylinder>(NAN_VAL),  std::invalid_argument);
    EXPECT_THROW(make_surface<Cylinder>(INF_VAL),  std::invalid_argument);
    EXPECT_THROW(make_surface<Cylinder>(-INF_VAL), std::invalid_argument);
}

TEST(Parabola, Validate)
{
    const double NAN_VAL = std::numeric_limits<double>::quiet_NaN();
    const double INF_VAL = std::numeric_limits<double>::infinity();

    EXPECT_NO_THROW(make_surface<Parabola>(1.0, 2.0));
    EXPECT_NO_THROW(make_surface<Parabola>(-1.0, -2.0));  // negative curvatures are valid
    EXPECT_NO_THROW(make_surface<Parabola>(0.0, 1.0));    // zero in one direction means flat
    EXPECT_NO_THROW(make_surface<Parabola>(1.0, 0.0));    // zero in one direction means flat

    EXPECT_THROW(make_surface<Parabola>(0.0, 0.0),           std::invalid_argument);  // use Flat
    EXPECT_THROW(make_surface<Parabola>(NAN_VAL, 1.0),       std::invalid_argument);
    EXPECT_THROW(make_surface<Parabola>(1.0, NAN_VAL),       std::invalid_argument);
    EXPECT_THROW(make_surface<Parabola>(INF_VAL, 1.0),       std::invalid_argument);
    EXPECT_THROW(make_surface<Parabola>(1.0, INF_VAL),       std::invalid_argument);
    EXPECT_THROW(make_surface<Parabola>(-INF_VAL, 1.0),      std::invalid_argument);
    EXPECT_THROW(make_surface<Parabola>(1.0, -INF_VAL),      std::invalid_argument);
}

TEST(Sphere, Validate)
{
    const double NAN_VAL = std::numeric_limits<double>::quiet_NaN();
    const double INF_VAL = std::numeric_limits<double>::infinity();

    EXPECT_NO_THROW(make_surface<Sphere>(0.001));
    EXPECT_NO_THROW(make_surface<Sphere>(1.0));

    EXPECT_THROW(make_surface<Sphere>(0.0),    std::invalid_argument);
    EXPECT_THROW(make_surface<Sphere>(-1.0),   std::invalid_argument);
    EXPECT_THROW(make_surface<Sphere>(NAN_VAL),  std::invalid_argument);
    EXPECT_THROW(make_surface<Sphere>(INF_VAL),  std::invalid_argument);
    EXPECT_THROW(make_surface<Sphere>(-INF_VAL), std::invalid_argument);
}
