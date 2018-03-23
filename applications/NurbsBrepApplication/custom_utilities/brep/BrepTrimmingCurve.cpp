//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:     BSD License
//               Kratos default license: kratos/IGAStructuralMechanicsApplication/license.txt
//
//  Main authors:    Tobias Teschemacher
//                   Michael Breitenberger
//

// System includes


// External includes 


// Project includes
#include "BrepTrimmingCurve.h"
#include "nurbs_brep_application.h"
#include "nurbs_brep_application_variables.h"


namespace Kratos
{
	/*
	* computes the polygon linearization of the NURBS-curve.
	* @param[in] number_of_polygon_points is the number of polygon points of the discretization.
	*/
	std::vector<array_1d<double, 2>> BrepTrimmingCurve::CreatePolygon(unsigned int number_polygon_points)
	{
		std::vector<array_1d<double, 2>> polygon;
		polygon.resize(number_polygon_points);

		// Variables needed
		unsigned int counter = 0;

		double u_min = m_knot_vector_u[0];
		double u_max = m_knot_vector_u[m_knot_vector_u.size() - 1];
		double delta_u = (u_max - u_min) / (number_polygon_points - 1);

		// Add points of edge to polygon
		double u_i = u_min - delta_u;
		for (unsigned int i = 0; i < number_polygon_points; i++)
		{
			u_i += delta_u;
			Point curve_point;

			EvaluateCurvePoint(curve_point, u_i);

			polygon[counter][0] = curve_point.X();
			polygon[counter][1] = curve_point.Y();

			counter++;
		}
		return polygon;
	}

	/*
	* Computes the polygon linearization of the NURBS-curve including the curve parameter.
	* Use for initial guess approximations.
	* @param[in] number_of_polygon_points is the number of polygon points of the discretization.
	*/
	std::vector<array_1d<double, 3>> BrepTrimmingCurve::CreatePolygonWithParameter(unsigned int number_polygon_points)
	{
		std::vector<array_1d<double, 3>> polygon;
		polygon.resize(number_polygon_points);

		double u_min = m_knot_vector_u(0);
		double u_max = m_knot_vector_u(m_knot_vector_u.size() - 1);
		double delta_u = (u_max - u_min) / number_polygon_points;

		// Add points of edge to polygon
		double u_i = u_min - delta_u;
		for (unsigned int i = 0; i < number_polygon_points; i++)
		{
			u_i += delta_u;
			Point curve_point;

			EvaluateCurvePoint(curve_point, u_i);

			polygon[i][0] = curve_point.X();
			polygon[i][1] = curve_point.Y();
			polygon[i][2] = u_i;
		}
		return polygon;
	}

	/* Computes the quadrature points on the curve, depending on the polynomial order polynomial_order_p.
	*  The amount of points do knot depend on the geometrical curve description.
	*/
	std::vector<array_1d<double, 3>> BrepTrimmingCurve::GetQuadraturePoints(std::vector<double> span, double polynomial_order_p)
	{
		std::vector<array_1d<double, 3>> quadrature_points;
		for (unsigned int i = 0; i < span.size() - 1; i++)
		{
			Vector parameter_u(2);
			parameter_u[0] = span[i];
			parameter_u[1] = span[i + 1];
			KnotSpan1d knot_span(0, polynomial_order_p, parameter_u);

			std::vector<array_1d<double, 2>> integration_points = knot_span.getIntegrationPointsInParameterDomain();
			for (unsigned int j = 0; j < integration_points.size(); j++)
			{
				Point integration_point;
				EvaluateCurvePoint(integration_point, integration_points[j][0]);

				array_1d<double, 3> quadrature_point;
				quadrature_point[0] = integration_point[0];
				quadrature_point[1] = integration_point[1];
				quadrature_point[2] = integration_points[j][1];

				quadrature_points.push_back(quadrature_point);
			}
		}
		return quadrature_points;
	}


	/*
	*  Computes the quadrature points on the curve, depending on the polynomial order polynomial_order_p.
	*  The amount of points do knot depend on the geometrical curve description.
	*  The percentage in the knot span is enhanced for facilitated projection towards other curves.
	*/
	std::vector<array_1d<double, 4>> BrepTrimmingCurve::GetIntegrationPoints(const std::vector<double>& rSpan, const double& rDegree)
	{
		std::vector<array_1d<double, 4>> integration_points_weight_percentage;
		for (unsigned int i = 0; i < rSpan.size() - 1; i++)
		{
			Vector parameter_u(2);
			parameter_u[0] = rSpan[i];
			parameter_u[1] = rSpan[i + 1];
			KnotSpan1d knot_span(0, rDegree, parameter_u);


			std::vector<array_1d<double, 2>> integration_points = knot_span.getIntegrationPointsInParameterDomain();
			for (unsigned int j = 0; j < integration_points.size(); j++)
			{
				Point integration_point;
				EvaluateCurvePoint(integration_point, integration_points[j][0]);

				array_1d<double, 3> quadrature_point;
				quadrature_point[0] = integration_point[0];
				quadrature_point[1] = integration_point[1];
				quadrature_point[2] = integration_points[j][1];

				double a = (m_active_range[1] - m_active_range[0]);
				double b = (integration_points[j][0] - m_active_range[0]);
				quadrature_point[3] = b/a;
				if (quadrature_point[3] < 0)
					KRATOS_ERROR << "ERROR: Wrong percentage!" << std::endl;


				integration_points_weight_percentage.push_back(quadrature_point);
			}
		}
		return integration_points_weight_percentage;
	}


	/* Computes all intersection parameters of the curve with the given knots in u and v direction. The geometrical
	*  curve description is not taken into account
	*  @param p, q polynomial degrees of underlaying surface for the amount of integration points. 
	*  @param knot_vector_u, knot_vector_v knot vectors of the underlaying surface for knot intersections
	*/
	std::vector<double> BrepTrimmingCurve::GetKnotIntersections(const int& p, const int& q,
		const Vector& knot_vector_u, const Vector& knot_vector_v, 
		const int& rNumberOfPolygonPoints = 100)
	{

		std::vector<array_1d<double, 3>> trim_polygon = CreatePolygonWithParameter(rNumberOfPolygonPoints);
		std::vector<double> intersections;

		intersections.push_back(m_active_range[0]);

		int span_u = NurbsUtilities::find_knot_span(p, knot_vector_u, trim_polygon[0][0]);
		int span_v = NurbsUtilities::find_knot_span(q, knot_vector_v, trim_polygon[0][1]);

		for (unsigned int i = 0; i < trim_polygon.size(); i++)
		{
			int new_span_u = NurbsUtilities::find_knot_span(p, knot_vector_u, trim_polygon[i][0]);
			if (span_u < new_span_u)
			{
				double parameter = trim_polygon[i - 1][2];
				bool success = GetKnotIntersection(parameter, 0, knot_vector_u[new_span_u]);
				if (success)
					intersections.push_back(parameter);
				else
				{
					success = GetKnotIntersectionBisection(parameter, trim_polygon[i - 1][2], trim_polygon[i][2], 0, knot_vector_u[new_span_u]);
					if (success)
						intersections.push_back(parameter);
					else
						KRATOS_ERROR << "Newton-Raphson and Bisection did not converge!" << std::endl;
				}
				span_u = new_span_u;
			}
			if (span_u > new_span_u)
			{
				double parameter = trim_polygon[i - 1][2];
				bool success = GetKnotIntersection(parameter, 0, knot_vector_u[span_u]);
				if (success)
					intersections.push_back(parameter);
				else
				{
					success = GetKnotIntersectionBisection(parameter, trim_polygon[i - 1][2], trim_polygon[i][2], 0, knot_vector_u[span_u]);
					if (success)
						intersections.push_back(parameter);
					else
						KRATOS_ERROR << "Newton-Raphson and Bisection did not converge!" << std::endl;
				}
				span_u = new_span_u;
			}

			int new_span_v = NurbsUtilities::find_knot_span(q, knot_vector_v, trim_polygon[i][1]);
			if (span_v < new_span_v)
			{
				double parameter = trim_polygon[i - 1][2];
				bool success = GetKnotIntersection(parameter, 1, knot_vector_v[new_span_v]);
				if (success)
					intersections.push_back(parameter);
				else
				{
					success = GetKnotIntersectionBisection(parameter, trim_polygon[i - 1][2], trim_polygon[i][2], 1, knot_vector_v[new_span_v]);
					if (success)
						intersections.push_back(parameter);
					else
						KRATOS_ERROR << "Newton-Raphson and Bisection did not converge!" << std::endl;
				}
				span_v = new_span_v;
			}
			if (span_v > new_span_v)
			{
				double parameter = trim_polygon[i - 1][2];
				bool success = GetKnotIntersection(parameter, 1, knot_vector_v[span_v]);
				if (success)
					intersections.push_back(parameter);
				else
				{
					success = GetKnotIntersectionBisection(parameter, trim_polygon[i - 1][2], trim_polygon[i][2], 1, knot_vector_v[span_v]);
					if (success)
						intersections.push_back(parameter);
					else
						KRATOS_ERROR << "Newton-Raphson and Bisection did not converge!" << std::endl;
				}
				span_v = new_span_v;
			}
		}
		intersections.push_back(m_active_range[1]);
		std::vector<double> full_intersections;
		std::sort(intersections.begin(), intersections.end());

		// delte multiple intersections
		//full_intersections.push_back(m_knot_vector_u[0]);
		//if (abs(m_knot_vector_u[0] - intersections[0]) > 1e-7 ||
		//	abs(m_knot_vector_u[m_knot_vector_u.size()-1] - intersections[0]) > 1e-7)
		full_intersections.push_back(intersections[0]);
		for (unsigned int j = 1; j < intersections.size(); j++)
		{
			if (intersections[j - 1] != intersections[j])
			{
				//if (abs(m_knot_vector_u[0] - intersections[j]) > 1e-7 ||
				//	abs(m_knot_vector_u[m_knot_vector_u.size()-1] - intersections[j]) > 1e-7)
				full_intersections.push_back(intersections[j]);
			}
		}
		//full_intersections.push_back(m_knot_vector_u[m_knot_vector_u.size()-1]);
		//std::sort(full_intersections.begin(), full_intersections.end());
		return full_intersections;
	}

	/* Parameters of closest points on curve are obtained
	*  @param intersection_points list of points for which the intersections are obtained
	*/
	std::vector<double> BrepTrimmingCurve::GetClosestPoints(std::vector<array_1d<double, 2>> points, const int& rNumberOfPolygonPoints = 100)
	{
		std::vector<array_1d<double, 3>> trim_polygon = CreatePolygonWithParameter(rNumberOfPolygonPoints);
		std::vector<double> parameters;
		for (unsigned int i = 0; i < points.size(); i++)
		{
			double parameter = 0;
			double distance = 1e10;
			for (int f = 0; f < trim_polygon.size(); ++f)
			{
				double new_distance = sqrt((points[i][0] - trim_polygon[f][0])*(points[i][0] - trim_polygon[f][0]) + (points[i][1] - trim_polygon[f][1])*(points[i][1] - trim_polygon[f][1]));
				if (new_distance < distance)
				{
					distance = new_distance;
					parameter = trim_polygon[f][2];
				}
			}
			if (!GetClosestPoint(points[i], parameter))
				std::cout << "Slave intersection point not found! Point: " << points[i][0] << ", " << points[i][1] << " with parameter: " << parameter << std::endl;
			parameters.push_back(parameter);
		}
		return parameters;
	}

	/* Parameter of closest point on curve is obtained. A percentage which is considererd to be the 
	*  percentage of the location in the knot span is passed.
	*  @param closest_point point for which the intersection is obtained
	*/
	bool BrepTrimmingCurve::GetClosestPoint(
		const array_1d<double, 2>& rClosestPoint, 
		const double& rKnotPercentage,
		double& rParameter)
	{
		bool success = false;
		rParameter = m_active_range[0] + rKnotPercentage*(m_active_range[1] - m_active_range[0]);
		success = ProjectionNewtonRaphson(rParameter, rClosestPoint);
		if (!success)
		{
			rParameter = m_active_range[0] + (1-rKnotPercentage)*(m_active_range[1] - m_active_range[0]);
			success = ProjectionNewtonRaphson(rParameter, rClosestPoint);
			if (!success)
			{
				success = ProjectionBisection(rParameter, rClosestPoint);
			}
		}
		return success;
	}

	/* Parameter of closest point on curve is obtained
	*  @param closest_point point for which the intersection is obtained
	*/
	bool BrepTrimmingCurve::GetClosestPoint(const array_1d<double, 2>& closest_point, double& parameter)
	{
		double ModelTolerance = 1e-8;

		double parameter_min = m_knot_vector_u(0);
		double parameter_max = m_knot_vector_u(m_knot_vector_u.size() - 1);
		if (parameter_min > parameter_max)
			std::swap(parameter_min, parameter_max);

		if (parameter < parameter_min)
			parameter = parameter_min;
		if (parameter > parameter_max)
			parameter = parameter_max;

		Matrix J = ZeroMatrix(2, 2);
		int itmax = 20;

		// START: Newton-Raphson
		for (unsigned int i = 0; i <= itmax; i++)
		{
			// establish residuum
			std::vector<Vector> derivatives;
			this->EvaluateCurveDerivatives(derivatives, 3, parameter);

			Vector difference(2);
			difference[0] = derivatives[0][0] - closest_point[0];
			difference[1] = derivatives[0][1] - closest_point[1];
			J = outer_prod(derivatives[0], difference);

			double orthogonal_projection = inner_prod(difference, derivatives[0]);
			double residual = difference[0] * derivatives[1][0] + difference[1] * derivatives[1][1];
			double jacobian = +sqrt(derivatives[1][0] * derivatives[1][0] + derivatives[1][1] * derivatives[1][1])
				+ (derivatives[2][0] * difference[0] + derivatives[2][1] * difference[1]);
			if (abs(orthogonal_projection) < 100 * ModelTolerance)
			{
				if (norm_2(difference) < 100 * ModelTolerance)
					return true;
			}

			if (abs(residual) < 10 * ModelTolerance)
				return true;

			if (norm_2(difference) < 10 * ModelTolerance)
				return true;

			// compute new iteration step
			parameter = parameter - residual / jacobian;

			// update solution
			if (parameter < parameter_min)
				parameter = parameter_min;
			if (parameter > parameter_max)
				parameter = parameter_max;
		}
		// END: Newton-Raphson
		return false;
	}

  //double BrepTrimmingCurve::EvaluateIntersection(double initial_u, int intersection_base, const double& coordinate_base)
  //{
  //  double local_parameter_u;
  //  bool converged;
  //  EvaluateLocalParameter(local_parameter_u, converged, intersection_base, coordinate_base, initial_u, 20, 1e-8);
  //  return local_parameter_u;
  //}
  //  #####################################################################################
  // #######################################################################################
  //#
  //#                  ++++++++++++++++++++++++++++++++++++++++++++++
  //#                  +++  Nurbs1DBasis::eval_Local_Parameter_Ex +++
  //#                  ++++++++++++++++++++++++++++++++++++++++++++++
  //#
  //   \details     returns the local parameter based one component of the position vector
  //                  (a*e11, b*e22, c*e33)
  //                  nonlinear equation is solved by means of a Newton Raphson scheme
  //
  // ======================================================================================
  //   \param[in]   _parameter      local parameter
  //   \param[in]   _converged      convergence flag
  //   \param[in]   _baseVec      base vector (1->e11, 2->e22, 3->e33)
  //   \param[in]   _baseComp        coordinate of the corresponding base vector(a, b, c)
  //   \param[in]  _uinit        Newton Raphson: inital guess
  //   \param[in]  _itmax        Newton Raphson: maximal number of iterations
  //   \param[in]  _iteps        Newton Raphson: tolerance
  //
  // ======================================================================================
  //  \author     A.Widhammer (11/2011)
  //
  //########################################################################################
  //void BrepTrimmingCurve::EvaluateLocalParameter(double& parameter, bool& converged, int baseVec,
  //  double baseComp, double uinit, double itmax, double iteps)
  //{
  //  std::cout << "Evaluate Local Parameter, u_initial: " << uinit << std::endl;
  //  // local parameters
  //  int i;
  //  double u_n;
  //  double u_np1;
  //  double dynR;
  //  double refR;
  //  double relR;
  //  double J;
  //  //array_1d<double, 2> bounds;
  //  Point point_1, point_2;
  //  //Point poi_max;
  //  Matrix dCdu;
  //  double Tolerance = 1e-9;
  //  if (fabs(baseComp) < 1e-9)
  //    baseComp = 0.0;
  //  // check span
  //  //this->min_Max_Knot(bounds);
  //  this->EvaluateCurvePoint(point_1, m_knot_vector_u(0));
  //  this->EvaluateCurvePoint(point_2, m_knot_vector_u(m_knot_vector_u.size()-1));
  //  double point_min = point_1(baseVec - 1);
  //  double parameter_min = m_knot_vector_u(0);
  //  double point_max = point_2(baseVec - 1);
  //  double parameter_max = m_knot_vector_u(m_knot_vector_u.size() - 1);
  //  if (point_min > point_max)
  //  {
  //    point_min = point_2(baseVec - 1);
  //    parameter_min = m_knot_vector_u(m_knot_vector_u.size() - 1);
  //    point_max = point_1(baseVec - 1);
  //    parameter_max = m_knot_vector_u(0);
  //  }
  //  if (baseComp<point_min)
  //  {
  //    parameter = parameter_min;
  //    converged = false;
  //  }
  //  else if (baseComp>point_max)
  //  {
  //    parameter = parameter_max;
  //    converged = false;
  //  }
  //  else
  //  {
  //    u_n = uinit;
  //    // START: Newton-Raphson
  //    for (i = 0; i <= itmax; i++)
  //    {
  //      // establish residuum
  //      Point point;
  //      this->EvaluateCurvePoint(point, u_n);
  //      array_1d<double, 2> base_vectors = this->GetBaseVector(u_n);//EvaluateCurveDerivatives(dCdu, 1, u_n);
  //      dynR = point(baseVec - 1) - baseComp;
  //      //KRATOS_WATCH(dynR)
  //      J = base_vectors(baseVec - 1);
  //      //KRATOS_WATCH(J)
  //      // check convergence - establish reference residuum
  //      if (i == 0)
  //      {
  //        refR = fabs(dynR);
  //        if (refR < 10 * Tolerance)
  //        {
  //          converged = true;
  //          parameter = u_n;
  //          break;
  //        }
  //      }
  //      relR = fabs(dynR);
  //      if ((relR < iteps) && (i != 0))
  //      {
  //        converged = true;
  //        parameter = u_n;
  //        break;
  //      }
  //      // compute new iteration step
  //      u_np1 = u_n - dynR / J;
  //      // update solution
  //      if (u_np1 < point_min)
  //      {
  //        u_n = parameter_min;
  //      }
  //      else if (u_np1 > point_max)
  //      {
  //        u_n = parameter_max;
  //      }
  //      else
  //      {
  //        u_n = u_np1;
  //      }
  //    }
  //    // END: Newton-Raphson
  //    if (i == itmax)
  //    {
  //      parameter = 0.00;
  //      converged = false;
  //    }
  //  }
  //}


/* Iterates the location of the intersection point with the knot.
*/
  bool BrepTrimmingCurve::GetKnotIntersectionBisection(
    double& parameter,
    const double& parameter_1,
    const double& parameter_2,
    const double& direction,
    const double& knot)
  {
    double parameter_smaller, parameter_bigger;
    Point point_1, point_2;
    this->EvaluateCurvePoint(point_1, parameter_1);
    this->EvaluateCurvePoint(point_2, parameter_2);

    double distance_1 = point_1(direction) - knot;
    if (distance_1 < 0)
    {
      parameter_smaller = parameter_1;
      parameter_bigger = parameter_2;
    }
    else
    {
      parameter_smaller = parameter_2;
      parameter_bigger = parameter_1;
    }

    int itmax = 50;
    for (int i = 0; i <= itmax; ++i)
    {
      double new_parameter = (parameter_smaller + parameter_bigger)/2;
      Point point_new;
      this->EvaluateCurvePoint(point_new, new_parameter);
      double new_distance = point_new(direction) - knot;
      if (abs(new_distance) < 1e-8)
      {
        parameter = new_parameter;
        return true;
      }
      if (new_distance < 0)
        parameter_smaller = new_parameter;
      else
        parameter_bigger = new_parameter;
    }
    return false;
  }

  /*
  *  returns the local parameter based one component of the position vector
  *                  (a*e11, b*e22, c*e33)
  *                  nonlinear equation is solved by means of a Newton Raphson scheme
  *
  *  @param[in/out] parameter   local parameter with initial guess
  *  @param[in]     direction   direction of knot
  *  @param[in]     knot        coordinate of the corresponding knot
  */
  bool BrepTrimmingCurve::GetKnotIntersection(
	  double& parameter, const int& direction, const double& knot)
  {
	  double parameter_init = parameter;
	  double Tolerance = 1e-9;

	  int itmax = 20;
	  // START: Newton-Raphson
	  for (int i = 0; i <= itmax; i++)
	  {
		  // establish residuum
		  std::vector<Vector> derivatives;
		  this->EvaluateCurveDerivatives(derivatives, 2, parameter);

		  double dynR = derivatives[0](direction) - knot;
		  // check convergence - establish reference residuum
		  if (i == 0)
		  {
			  double refR = fabs(dynR);
			  if (refR < 10 * Tolerance)
				  return true;
		  }
		  double relR = fabs(dynR);
		  if ((relR < 1e-8) && (i != 0))
			  return true;

		  // compute new iteration step
		  parameter = parameter - (dynR / derivatives[1](direction));
	  }
	  // END: Newton-Raphson
	  std::cout << "Newton-Raphson did not converge. Input was: dimension=" << direction << ", knot=" << knot << std::endl;

	  parameter = parameter_init;
	  return false;
  }

  bool BrepTrimmingCurve::ProjectionNewtonRaphson(double& parameter, const array_1d<double, 2>& closest_point)
  {
    double initial_param = parameter;
    int itmax = 20;
    double model_tolerance = 1e-3;
    double accuracy = 1e-8;

    Vector closest_point_vector(closest_point);

    double parameter_min = std::min(m_knot_vector_u(0), m_knot_vector_u(m_knot_vector_u.size() - 1));
    double parameter_max = std::max(m_knot_vector_u(0), m_knot_vector_u(m_knot_vector_u.size() - 1));

    if (parameter < parameter_min)
      parameter = parameter_min;
    if (parameter > parameter_max)
      parameter = parameter_max;

    for (unsigned int i = 0; i <= itmax; i++)
    {
      // derivatives: [0] location, [1] base vector, [2] curvature
      std::vector<Vector> derivatives;
      this->EvaluateCurveDerivatives(derivatives, 3, parameter);

      Vector distance = closest_point_vector - derivatives[0];
	  //KRATOS_WATCH(distance)
	  //KRATOS_WATCH(derivatives[0])
	  //KRATOS_WATCH(closest_point_vector)
      double orthogonal_projection = inner_prod(distance, derivatives[0]);
      if (abs(orthogonal_projection) < 100 * accuracy)
      {
        if (norm_2(distance) < model_tolerance)
        {
          return true;
        }
      }

      double residual = distance[0] * derivatives[1][0] + distance[1] * derivatives[1][1];

      if (norm_2(derivatives[1]) < 10000 * accuracy)
      {
        std::cout << "You got ... a singularity" << std::endl;
        KRATOS_ERROR << "no" << std::endl;
      }
      if (abs(residual) < 100 * accuracy)
      {
        if (norm_2(distance) < model_tolerance)
        {
          return true;
        }
      }

	  //KRATOS_WATCH(derivatives[1])
	  //KRATOS_WATCH(derivatives[2])
	  //KRATOS_WATCH(distance)

      double jacobian = - norm_2(derivatives[1])*norm_2(derivatives[1]) + (derivatives[2][0] * distance[0] + derivatives[2][1] * distance[1]);
      
	  //KRATOS_WATCH(jacobian)
		 // KRATOS_WATCH(residual)

      // compute new iteration step
      parameter = parameter - residual / jacobian;

      if (parameter < parameter_min)
        parameter = parameter_min;
      if (parameter > parameter_max)
        parameter = parameter_max;
    }
    std::cout << "Newton Raphson did not converge" << std::endl;
    KRATOS_WATCH(m_p)
    KRATOS_WATCH(m_knot_vector_u)
    KRATOS_WATCH(closest_point)
    KRATOS_WATCH(initial_param)
    for (int i = 0; i < m_control_points.size(); ++i)
    {
      KRATOS_WATCH(m_control_points[i])
    }
    //KRATOS_ERROR << "not converged" << std::endl;
    return true;
  }

  bool BrepTrimmingCurve::ProjectionBisection(
    double& parameter,
    const array_1d<double, 2>& closest_point)
  {
    double parameter_min = m_knot_vector_u(0);
    double parameter_max = m_knot_vector_u(m_knot_vector_u.size() - 1);

    if (parameter_min > parameter_max)
      std::swap(parameter_min, parameter_max);

    //if ((parameter < parameter_min)|| (parameter > parameter_max))
    //  return ProjectionBisection(parameter, closest_point, parameter_min, parameter_max);
    //Vector closest_point_vector(closest_point);
    //std::vector<Vector> derivatives;
    //this->EvaluateCurveDerivatives(derivatives, 1, parameter);
    //Vector distance = derivatives[0] - closest_point_vector;
    //double residual = distance[0] * derivatives[1][0] + distance[1] * derivatives[1][1];
    //if (residual < 0)
    //{
    //  parameter_min = parameter;
	//
    //  //parameter_max = parameter + 2 * residual;
    //}
    //else
    //{
    //  parameter_max = parameter;
	//
    //  //parameter_min = parameter - 2 * residual;
    //}

    return ProjectionBisection(parameter, closest_point, parameter_min, parameter_max);
  }

  bool BrepTrimmingCurve::ProjectionBisection(
    double& parameter,
    const array_1d<double, 2>& closest_point,
    double parameter_min,
    double parameter_max)
  {
    int itmax = 50;
    double model_tolerance = 1e-1;
    double accuracy = 1e-8;

    if (parameter_min > parameter_max)
      std::swap(parameter_min, parameter_max);
	
    Vector closest_point_vector(closest_point);

    std::vector<Vector> point_1_derivatives, point_2_derivatives;
    this->EvaluateCurveDerivatives(point_1_derivatives, 0, parameter_min);
    this->EvaluateCurveDerivatives(point_2_derivatives, 0, parameter_max);

    for (int i = 0; i <= itmax; ++i)
    {
      parameter = (parameter_min + parameter_max) / 2;
      std::vector<Vector> derivatives;
      this->EvaluateCurveDerivatives(derivatives, 1, parameter);
      Vector distance = derivatives[0] - closest_point_vector;

      double orthogonal_projection = inner_prod(distance, derivatives[0]);
      double residual = distance[0] * derivatives[1][0] + distance[1] * derivatives[1][1];
      double new_distance = norm_2(distance);

      //KRATOS_WATCH(parameter)

      if (abs(residual) < 1000 * accuracy)
      {
        if (norm_2(distance) < model_tolerance)
        {
          return true;
        }
      }
      if (abs(orthogonal_projection) < 1000*accuracy)
      {
        if (norm_2(distance) < 10*model_tolerance)
        {
          //std::cout << "orthogonal_projection break" << std::endl;
          return true;
        }
      }

      if (residual < 0)
        parameter_min = parameter;
      else
        parameter_max = parameter;
    }
    return false;
  }

  ////to be deleted
  //bool BrepTrimmingCurve::GetClosestPoint(const array_1d<double, 2>& closest_point, double& parameter)
  //{
  //  double ModelTolerance = 1e-8;
  //
  //  double parameter_min = m_knot_vector_u(0);
  //  double parameter_max = m_knot_vector_u(m_knot_vector_u.size() - 1);
  //  if (parameter_min > parameter_max)
  //    std::swap(parameter_min, parameter_max);
  //
  //  std::cout << "parameter_min: " << parameter_min << ", parameter_max: " << parameter_max << std::endl;
  //
  //  if (parameter < parameter_min)
  //  {
  //    parameter = parameter_min;
  //  }
  //  if (parameter > parameter_max)
  //  {
  //    parameter = parameter_max;
  //  }
  //
  //  Matrix J = ZeroMatrix(2,2);
  //  int itmax = 20;
  //  // START: Newton-Raphson
  //  for (unsigned int i = 0; i <= itmax; i++)
  //  {
  //    // establish residuum
  //    std::vector<Vector> derivatives;
  //    this->EvaluateCurveDerivatives(derivatives, 3, parameter);
  //    //KRATOS_WATCH(point)
  //    //KRATOS_WATCH(derivatives[0])
  //    //KRATOS_WATCH(base_vector)
  //    //KRATOS_WATCH(derivatives[1])
  //    Vector difference(2);
  //    difference[0] = derivatives[0][0] - closest_point[0];
  //    difference[1] = derivatives[0][1] - closest_point[1];
  //    J = outer_prod(derivatives[0], difference);
  //    //KRATOS_WATCH(parameter)
  //    //KRATOS_WATCH(J)
  //    //J = base_vectors(baseVec - 1);
  //    //KRATOS_WATCH(base_vector)
  //    //double determinant = (J(0, 0)*J(1, 1) - J(0, 1)*J(1, 0));
  //    double orthogonal_projection = inner_prod(difference, derivatives[0]);
  //    double residual = difference[0] * derivatives[1][0] + difference[1] * derivatives[1][1];
  //    double jacobian = + sqrt(derivatives[1][0] * derivatives[1][0] + derivatives[1][1] * derivatives[1][1])
  //      + (derivatives[2][0]*difference[0] + derivatives[2][1] * difference[1]);
  //    if (abs(orthogonal_projection) < 100 * ModelTolerance)
  //    {
  //      if (norm_2(difference) < 100 * ModelTolerance)
  //      {
  //        std::cout << "orthogonal_projection break" << std::endl;
  //        return true;
  //      }
  //    }
  //    //KRATOS_WATCH(residual)
  //    //KRATOS_WATCH(derivatives[2])
  //    if (abs(residual) < 10 * ModelTolerance)
  //    {
  //      std::cout << "Residual break" << std::endl;
  //      return true;
  //    }
  //    if (norm_2(difference) < 10 * ModelTolerance)
  //    {
  //      std::cout << "Difference break" << std::endl;
  //      return true;
  //    }
  //    // compute new iteration step
  //    parameter = parameter - residual/jacobian;
  //    // update solution
  //    if (parameter < parameter_min)
  //    {
  //      parameter = parameter_min;
  //    }
  //    if (parameter > parameter_max)
  //    {
  //      parameter = parameter_max;
  //    }
  //    // END: Newton-Raphson
  //    //if (i == itmax)
  //    //{
  //    //  parameter = 0.00;
  //    //  KRATOS_THROW_ERROR(std::logic_error, "BrepTrimmingCurve: Newton Raphson failed! Point u: " + std::to_string(closest_point[0]) + " v: " , std::to_string(closest_point[1]));
  //    //KRATOS_ERROR_IF(i == itmax) << "BrepTrimmingCurve: Newton Raphson failed! Point u: " << closest_point[0] << " v: " << std::to_string(closest_point[1]) << std::endl;
  //    //}
  //  }
  //  return false;
  //}
  //////to be deleted
  //bool BrepTrimmingCurve::GetClosestPointBisection(
  //  double& parameter,
  //  const array_1d<double, 2>& closest_point)
  //{
  //  double parameter_min = m_knot_vector_u(0);
  //  double parameter_max = m_knot_vector_u(m_knot_vector_u.size() - 1);
  //  if (parameter_min > parameter_max)
  //    std::swap(parameter_min, parameter_max);
  //
  //  //double parameter_smaller, parameter_bigger;
  //  Point point_1, point_2;
  //  this->EvaluateCurvePoint(point_1, parameter_min);
  //  this->EvaluateCurvePoint(point_2, parameter_max);
  //
  //  Vector distance = ZeroVector(2);
  //  //double distance_1 = point_1(direction) - knot;
  //  //if (distance_1 < 0)
  //  //{
  //  //  parameter_smaller = parameter_min;
  //  //  parameter_bigger = parameter_max;
  //  //}
  //  //else
  //  //{
  //  //  parameter_smaller = parameter_max;
  //  //  parameter_bigger = parameter_min;
  //  //}
  //  int itmax = 50;
  //  for (int i = 0; i <= itmax; ++i)
  //  {
  //    double new_parameter = (parameter_min + parameter_max) / 2;
  //    Point point_new;
  //    this->EvaluateCurvePoint(point_new, new_parameter);
  //    distance[0] = point_new[0] - closest_point[0];
  //    distance[1] = point_new[1] - closest_point[1];
  //    double orthogonal_projection = inner_prod(distance, point_new);
  //    std::vector<Vector> derivatives;
  //    this->EvaluateCurveDerivatives(derivatives, 2, parameter);
  //
  //    //array_1d<double, 2> base_vector = this->GetBaseVector(parameter);//EvaluateCurveDerivatives(dCdu, 1, u_n);
  //    double residual = distance[0] * derivatives[1][0] + distance[1] * derivatives[1][1];
  //    double new_distance = norm_2(distance);
  //    KRATOS_WATCH(new_parameter)
  //    if (abs(residual) < 1e-6)
  //    {
  //      parameter = new_parameter;
  //      return true;
  //    }
  //    if (abs(orthogonal_projection) < 1e-6)
  //    {
  //      parameter = new_parameter;
  //      return true;
  //    }
  //    if (residual < 0)
  //      parameter_min = new_parameter;
  //    else
  //      parameter_max = new_parameter;
  //  }
  //  return false;
  //}
  //array_1d<double, 2> BrepTrimmingCurve::GetBaseVector(const int& u)
  //{
  //  int span = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, u);
  //  Matrix N;
  //  NurbsUtilities::eval_nonzero_basis_function_with_derivatives(N, m_knot_vector_u, u, span, m_p, 3);
  //  //this->eval_Derivative_NonzeroBasis_Fct(N, U_Vec, _u, i, P_Deg, 1);
  //  double sum = 0.0;
  //  double dsum = 0.0;
  //  Vector R(m_p + 1);
  //  Vector dR(m_p + 1);
  //  double weight;
  //  for (int b = 0; b <= m_p; b++)
  //  {
  //    weight = m_control_points[span - m_p + b][3];
  //    R(b) = N(0, b)*weight;
  //    sum += R(b);
  //    // derivatives
  //    dR(b) = N(1, b)*weight;
  //    dsum += dR(b);
  //  }
  //  // divide by sum only required in terms of rational basis functions
  //  if (fabs(sum - weight)> 0.00000001)
  //  {
  //    double inv_sum = 1 / sum;
  //    double inv_sum_2 = 1 / pow(sum, 2);
  //    // divide through by sum
  //    for (int b = 0; b <= m_p; b++)
  //    {
  //      dR(b) = inv_sum*dR(b) - R(b)*dsum*inv_sum_2;
  //      R(b) = inv_sum*R(b);
  //    }
  //  }
  //  array_1d<double, 2> g;
  //  g.clear();
  //  for (int b = 0; b <= m_p; b++)
  //  {
  //    g(0) += dR(b)*m_control_points[span - m_p + b][0];
  //    g(1) += dR(b)*m_control_points[span - m_p + b][1];
  //    //g(2) += dR(b)*Ctrl_Pt[i-P_Deg+b]->get_Coo_e33();
  //  }
  //  return g;
  //}

  /*
  * Evaluates the location of a point given by the curve parameter.
  */
  void BrepTrimmingCurve::EvaluateCurvePoint(Point& rCurvePoint, double parameter_u)
  {
	  std::vector<Vector> derivatives;
	  EvaluateCurveDerivatives(derivatives, 1, parameter_u);

	  rCurvePoint[0] = derivatives[0](0);
	  rCurvePoint[1] = derivatives[0](1);
	  rCurvePoint[2] = 0;
  }

  /*
  * Computes the rational location, base vectors and curve derivatives at a specific with free order
  * @param[out] derivatives [0] location
  *                         [1] base vectors
  *                         [2] curvature
  * @param[in]  order of derivatives
  * @param[in]  u     parameter
  */
  void BrepTrimmingCurve::EvaluateCurveDerivatives(std::vector<Vector>& derivatives, const int& order, const double& u)
  {
	  std::vector<Vector> DN_De;
	  EvaluateRationalCurveDerivatives(DN_De, order, u);
	  for (int o = 0; o <= order; o++)
	  {
		  Vector derivative = ZeroVector(2);
		  for (int k = 0; k < m_control_points.size(); k++)
		  {
			  derivative[0] += DN_De[o][k] * m_control_points[k][0] * m_control_points[k][3];
			  derivative[1] += DN_De[o][k] * m_control_points[k][1] * m_control_points[k][3];
		  }
		  derivatives.push_back(derivative);
	  }
  }

  /*
  * Computes the rational curve shape functions and derivatives
  * Adapted from algorithm in Piegl, Les. et al. "The NURBS Book".
  * Algorithm A4.2
  * @param[out] DN_De 
  * @param[in]  order of derivatives
  * @param[in]  u     parameter
  */
  void BrepTrimmingCurve::EvaluateRationalCurveDerivatives(std::vector<Vector>& DN_De, const int& order, const double& u)
  {
    int span = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, u);

    // 1. Compute the BSpline basis functions and their derivatives at u
    Matrix dN;
    NurbsUtilities::eval_nonzero_basis_function_with_derivatives(dN, m_knot_vector_u, u, span, m_p, order);

    Vector dF = ZeroVector(order + 1);

    // Compute Denominator
    for (int i = 0; i <= order; ++i)
    {
      // 1. Loop over all the basis functions
      for (int j = 0; j <= m_p; ++j)
      {
        // 1.1.Get the Control Point index
        int index = span - m_p + j;
        // 1.2.Compute the denominator function iteratively
        dF(i) = dF(i) + dN(i, j)*m_control_points[index][3];
      }
    }

    // 3i.Loop over all the derivatives
    for (int k = 0; k <= order; k++)
    { 
      Vector dR = ZeroVector(m_control_points.size());
      DN_De.push_back(dR);
      // 3. Loop over all the basis functions
      for (int j = 0; j <= m_p; ++j)
      {
        // 3.2. Compute the Control point index
        int index = span - m_p + j;
        // 3.3. Compute the product of the derivatives of the basis functions with the Control Point weights
        double v = dN(k, j)*m_control_points[index][3];
        // 3.4. Loop over all the involved derivatives
        for (int i = 1; i <= k; ++i)
        {
          v = v - NurbsUtilities::binom(k, i)*dF[i]*DN_De[k - i][j];
        }
        // 3.5. Divide by the denominator function
        DN_De[k][index] = v / dF[0];
      }
    }
  }

  //void BrepTrimmingCurve::EvaluateRationalCurveDerivativesPrint(std::vector<Vector>& DN_De, const int& order, const double& u)
  //{
  //  int span = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, u);
  //  KRATOS_WATCH(u)
  //  KRATOS_WATCH(m_knot_vector_u)
  //    KRATOS_WATCH(m_p)
  //    for (int i = 0; i < m_control_points.size(); i++)
  //    {
  //      std::cout << m_control_points[i][3];
  //    }
  //  std::cout << std::endl;
  //  // 1. Compute the BSpline basis functions and their derivatives at u
  //  // BSpline basis functions and their derivatives
  //  Matrix dN;
  //  NurbsUtilities::eval_nonzero_basis_function_with_derivatives(dN, m_knot_vector_u, u, span, m_p, order);
  //  //KRATOS_WATCH(dN)
  //  Vector dF = ZeroVector(order + 1);
  //  // computeDenominator
  //  for (int i = 0; i <= order; ++i)
  //  {
  //    // 1i.Loop over all the basis functions
  //    for (int j = 0; j <= m_p; ++j)// m_control_points.size(); ++j)
  //    {
  //      // 1i.1.Get the Control Point index
  //      int index = span - m_p + j;
  //      //KRATOS_WATCH(index)
  //      // 1i.2.Compute the denominator function iteratively
  //      dF(i) = dF(i) + dN(i, j)*m_control_points[index][3];
  //    }
  //  }
  //  KRATOS_WATCH(dF)
  //  // 3i.Loop over all the derivatives
  //  for (int k = 0; k <= order; k++)
  //  {
  //    Vector dR = ZeroVector(m_control_points.size());
  //    DN_De.push_back(dR);
  //    // 3. Loop over all the basis functions
  //    for (int j = 0; j <= m_p; ++j)// m_control_points.size(); j++)
  //    {
  //      // 3i.2. Compute the Control point index
  //      int index = span - m_p + j;
  //      //KRATOS_WATCH(index)
  //      // 3i.3. Compute the product of the derivatives of the basis functions with the Control Point weights
  //      double v = dN(k, j)*m_control_points[index][3];
  //      //KRATOS_WATCH(dN.size2())
  //      // 3i.4. Loop over all the involved derivatives
  //      for (int i = 1; i <= k; ++i)
  //      {
  //        //KRATOS_WATCH(i)
  //        //KRATOS_WATCH(NurbsUtilities::binom(k, i))
  //        //KRATOS_WATCH(k)
  //        //KRATOS_WATCH(dF[i])
  //        //KRATOS_WATCH(DN_De[k - i][j])
  //        v = v - NurbsUtilities::binom(k, i)*dF[i] * DN_De[k - i][j];
  //      }
  //      // 3i.5. Divide by the denominator function
  //      DN_De[k][j] = v / dF[0];
  //    }
  //  }
  //  for (int k = 0; k <= order; ++k)
  //  {
  //    KRATOS_WATCH(DN_De[k])
  //  }
  //}
  //void BrepTrimmingCurve::EvaluateCurveDerivativesPrint(Matrix& DN_De, const int& order, const double& u)
  //{
  //  //DN_De = ZeroMatrix(m_control_points.size(), order);
  //  //for (unsigned int k = 0; k <= order; k++)
  //  //{
  //  //  for (unsigned int n_cp_itr = 0; n_cp_itr <= DN_De.size1(); n_cp_itr++)
  //  //  {
  //  //    DN_De(n_cp_itr, k) = m_control_points[n_cp_itr][k];
  //  //  }
  //  //  for (unsigned int i = 1; i <= k; i++)
  //  //  {
  //  //    for (unsigned int n_cp_itr = 0; n_cp_itr <= DN_De.size1(); n_cp_itr++)
  //  //    {
  //  //      DN_De(n_cp_itr, k) = DN_De(n_cp_itr, k) - NurbsUtilities::binom(k,i)*DN_De(n_cp_itr,k-i);
  //  //    }
  //  //  }
  //  //}
  //  DN_De.resize(4, m_control_points.size());
  //  matrix<double> N;                        //B-Spline basis functions
  //  int span = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, u);
  //  NurbsUtilities::eval_nonzero_basis_function_with_derivatives(N, m_knot_vector_u, u, span, m_p, 3);
  //  double sum = 0;    //sum of weights times basis functions
  //  double dsum = 0;   //sum of weights times 1st derivative of basis functions
  //  double ddsum = 0;  //sum of weights times 2nd derivative of basis functions
  //  double dddsum = 0; //sum of weights times 3rd derivative of basis functions
  //  for (int i = 0; i <= m_p; i++)
  //  {
  //    double weight = m_control_points[i + span - m_p][3];
  //    DN_De(0, i) = N(0, i)*weight;
  //    sum += DN_De(0, i);
  //    DN_De(1, i) = N(1, i)*weight;
  //    dsum += DN_De(1, i);
  //    DN_De(2, i) = N(2, i)*weight;
  //    ddsum += DN_De(2, i);
  //    DN_De(3, i) = N(3, i)*weight;
  //    dddsum += DN_De(3, i);
  //  }
  //  KRATOS_WATCH(sum)
  //  KRATOS_WATCH(dsum)
  //  KRATOS_WATCH(ddsum)
  //  KRATOS_WATCH(dddsum)
  //  //get derivatives
  //  for (int i = 0; i <= m_p; i++)
  //  {
  //    //3rd derivative
  //    DN_De(3, i) = DN_De(3, i) / sum - 3 * DN_De(2, i)*dsum / pow(sum, 2) + 4 * DN_De(1, i)*pow(dsum, 2) / pow(sum, 3)
  //      - (3 * DN_De(1, i)*ddsum + DN_De(0, i)*dddsum) / pow(sum, 2) + 2 * DN_De(0, i)*ddsum*dsum / pow(sum, 3)
  //      + (2 * DN_De(1, i)*pow(dsum, 2) + 4 * DN_De(0, i)*dsum*ddsum) / pow(sum, 3)
  //      - 6 * DN_De(0, i)*pow(dsum, 3) / pow(sum, 4);
  //    //DN_De(3,i) = DN_De(3,i)/sum - 3*DN_De(2,i)*dsum/pow(sum,2) - 3*DN_De(1,i)*ddsum/pow(sum,2) + 4*DN_De(1,i)*dsum*ddsum/pow(sum,3)
  //    //         - DN_De(0,i)*dddsum/pow(sum,2) + 2*DN_De(1,i)*pow(dsum,2)/pow(sum,3) + 6*DN_De(0,i)*dsum*ddsum/pow(sum,3) - 6*pow(dsum,3)/pow(sum,4);
  //    //2nd derivative
  //    DN_De(2, i) = DN_De(2, i) / sum - 2 * DN_De(1, i)*dsum / pow(sum, 2) - DN_De(0, i)*ddsum / pow(sum, 2) + 2 * DN_De(0, i)*pow(dsum, 2) / pow(sum, 3);
  //    //1st derivative
  //    DN_De(1, i) = DN_De(1, i) / sum - DN_De(0, i)*dsum / pow(sum, 2);
  //    //basis functions
  //    DN_De(0, i) = DN_De(0, i) / sum;
  //  }
  //  KRATOS_WATCH(N)
  //  KRATOS_WATCH(DN_De)
  //}
  //to be deleted
  //void BrepTrimmingCurve::EvaluateCurveDerivatives(Matrix& DN_De, const int& order, const double& u)
  //{
  //  //DN_De = ZeroMatrix(m_control_points.size(), order);
  //  //for (unsigned int k = 0; k <= order; k++)
  //  //{
  //  //  for (unsigned int n_cp_itr = 0; n_cp_itr <= DN_De.size1(); n_cp_itr++)
  //  //  {
  //  //    DN_De(n_cp_itr, k) = m_control_points[n_cp_itr][k];
  //  //  }
  //  //  for (unsigned int i = 1; i <= k; i++)
  //  //  {
  //  //    for (unsigned int n_cp_itr = 0; n_cp_itr <= DN_De.size1(); n_cp_itr++)
  //  //    {
  //  //      DN_De(n_cp_itr, k) = DN_De(n_cp_itr, k) - NurbsUtilities::binom(k,i)*DN_De(n_cp_itr,k-i);
  //  //    }
  //  //  }
  //  //}
  //  DN_De.resize(4, m_control_points.size());
  //  matrix<double> N;                        //B-Spline basis functions
  //  int span = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, u);
  //  NurbsUtilities::eval_nonzero_basis_function_with_derivatives(N, m_knot_vector_u, u, span, m_p, 3);
  //  
  //  double sum = 0;    //sum of weights times basis functions
  //  double dsum = 0;   //sum of weights times 1st derivative of basis functions
  //  double ddsum = 0;  //sum of weights times 2nd derivative of basis functions
  //  double dddsum = 0; //sum of weights times 3rd derivative of basis functions
  //  for (int i = 0; i <= m_p; i++)
  //  {
  //    double weight = m_control_points[i + span - m_p][3];
  //    DN_De(0, i) = N(0, i)*weight;
  //    sum += DN_De(0, i);
  //    DN_De(1, i) = N(1, i)*weight;
  //    dsum += DN_De(1, i);
  //    DN_De(2, i) = N(2, i)*weight;
  //    ddsum += DN_De(2, i);
  //    DN_De(3, i) = N(3, i)*weight;
  //    dddsum += DN_De(3, i);
  //  }
  //  //get derivatives
  //  for (int i = 0; i <= m_p; i++)
  //  {
  //    //3rd derivative
  //    DN_De(3, i) = DN_De(3, i) / sum - 3 * DN_De(2, i)*dsum / pow(sum, 2) + 4 * DN_De(1, i)*pow(dsum, 2) / pow(sum, 3)
  //      - (3 * DN_De(1, i)*ddsum + DN_De(0, i)*dddsum) / pow(sum, 2) + 2 * DN_De(0, i)*ddsum*dsum / pow(sum, 3)
  //      + (2 * DN_De(1, i)*pow(dsum, 2) + 4 * DN_De(0, i)*dsum*ddsum) / pow(sum, 3)
  //      - 6 * DN_De(0, i)*pow(dsum, 3) / pow(sum, 4);
  //    //DN_De(3,i) = DN_De(3,i)/sum - 3*DN_De(2,i)*dsum/pow(sum,2) - 3*DN_De(1,i)*ddsum/pow(sum,2) + 4*DN_De(1,i)*dsum*ddsum/pow(sum,3)
  //    //         - DN_De(0,i)*dddsum/pow(sum,2) + 2*DN_De(1,i)*pow(dsum,2)/pow(sum,3) + 6*DN_De(0,i)*dsum*ddsum/pow(sum,3) - 6*pow(dsum,3)/pow(sum,4);
  //    //2nd derivative
  //    DN_De(2, i) = DN_De(2, i) / sum - 2 * DN_De(1, i)*dsum / pow(sum, 2) - DN_De(0, i)*ddsum / pow(sum, 2) + 2 * DN_De(0, i)*pow(dsum, 2) / pow(sum, 3);
  //    //1st derivative
  //    DN_De(1, i) = DN_De(1, i) / sum - DN_De(0, i)*dsum / pow(sum, 2);
  //    //basis functions
  //    DN_De(0, i) = DN_De(0, i) / sum;
  //  }
  //  //KRATOS_WATCH(N)
  //  //KRATOS_WATCH(DN_De)
  //}


	/* Returns trimmin curve index
	*/
	unsigned int& BrepTrimmingCurve::GetIndex()
	{
		return m_trim_index;
	}

  /**
  * This functions inserts knots at the positions rRu.
  * Adapted from algorithm in Piegl, Les. et al. "The NURBS Book".
  * Algorithm A5.4 by Boehm and Prautzsch
  *
  * @param rRu: knots to be inserted
  */
  void BrepTrimmingCurve::RefineKnotVector(const Vector& rRu)
  {
	  KRATOS_ERROR << "RefineKnotVector implemented but not tested yet!" << std::endl;

	  if (rRu.size() == 0)
		  KRATOS_ERROR << "Error: No knot for refinement." << std::endl;

	  std::vector<double> Ru; //reduced vectors of knots that do not exceed the borders
	  double tolerance = 10e-8;
	  for (int i = 0; i < rRu.size(); i++) // 
	  {
		  if ((m_knot_vector_u[0] - rRu[i]) <tolerance && (rRu[i] - m_knot_vector_u[m_knot_vector_u.size() - 1]) < tolerance)
			  Ru.push_back(rRu[i]);
	  }

	  std::vector<array_1d<double, 4>> Qw(m_control_points.size() + Ru.size());

	  int m = m_control_points.size() + m_p + 1;
	  int a = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, Ru[0]);
	  int b = NurbsUtilities::find_knot_span(m_p, m_knot_vector_u, Ru[Ru.size()-1]);
	  b += 1;

	  for (int j = 0; j <= a - m_p; j++)
		  Qw[j] = m_control_points[j];
	  for (int j = b - 1; j <= m_control_points.size(); j++)
		  Qw[j+ Ru.size()] = m_control_points[j];

	  Vector knot_vector = ZeroVector(m + Ru.size() + 1);

	  for (int j = 0; j <= a; j++)
		  knot_vector[j] = m_knot_vector_u[j];
	  for (int j = b + m_p; j <= m; j++)
		  knot_vector[j + m_p + 1] = m_knot_vector_u[j];

	  int index_i = b + m_p - 1;
	  int index_k = b + m_p + Ru.size();

	  for (int j = Ru.size(); j >= 0; j--)
	  {
		  while (Ru[j] <= m_knot_vector_u[index_i] && index_i > a)
		  {
			  Qw[index_k - m_p - 1] = m_control_points[index_i - m_p - 1];
			  knot_vector[index_k] = m_knot_vector_u[index_i];
			  index_k = index_k - 1;
			  index_i = index_i - 1;
		  }
		  Qw[index_k - m_p - 1] = Qw[index_k - m_p];
		  for (int l = 1; l <= m_p; l++)
		  {
			  int index = index_k - m_p + 1;
			  int alfa = knot_vector[index_k + 1] - Ru[j];
			  if (abs(alfa) == 0.0)
				  Qw[index - 1] = Qw[index];
			  else
			  {
				  alfa = alfa / (knot_vector[index_k + l] - m_knot_vector_u[index_i - m_p + l]);
				  Qw[index - 1] = alfa*Qw[index - 1] + (1.0 - alfa)*Qw[index];
			  }
		  }
		  knot_vector[index_k] = Ru[j];
		  index_k = index_k - 1;
	  }
	  m_knot_vector_u = knot_vector;
	  m_control_points = Qw;
  }

	//Constructor
	BrepTrimmingCurve::BrepTrimmingCurve(unsigned int trim_index, bool curve_direction, Vector& knot_vector_u,
		unsigned int p, ControlPointVector& control_points,
		Vector& active_range)
		: m_knot_vector_u(knot_vector_u),
		m_curve_direction(curve_direction),
		m_p(p),
		m_control_points(control_points),
		m_active_range(active_range),
		m_trim_index(trim_index)
	{
	}
	//Destructor
  BrepTrimmingCurve::~BrepTrimmingCurve()
  {}

}  // namespace Kratos.

