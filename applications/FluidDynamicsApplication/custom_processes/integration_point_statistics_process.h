//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//

#if !defined(KRATOS_INTEGRATION_POINT_STATISTICS_PROCESS_H_INCLUDED)
#define KRATOS_INTEGRATION_POINT_STATISTICS_PROCESS_H_INCLUDED

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "includes/define.h"
#include "includes/kratos_parameters.h"
#include "processes/process.h"
#include "custom_utilities/statistics_record.h"

namespace Kratos
{
///@addtogroup ApplicationNameApplication
///@{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
  */
class IntegrationPointStatisticsProcess: public Process
{
public:
///@name Type Definitions
///@{

/// Pointer definition of IntegrationPointStatisticsProcess
KRATOS_CLASS_POINTER_DEFINITION(IntegrationPointStatisticsProcess);

///@}
///@name Life Cycle
///@{

/// Default constructor.
IntegrationPointStatisticsProcess(ModelPart& rModelPart, Kratos::Parameters Parameters):
    Process(),
    mrModelPart(rModelPart),
    mParameters(Parameters),
    mStartTime(0.0)
{}

/// Destructor.
~IntegrationPointStatisticsProcess() override
{}

///@}
///@name Operators
///@{

///@}
///@name Operations
///@{

void ExecuteInitialize() override
{
    KRATOS_TRY;

    // Validate parameters
    const Kratos::Parameters default_parameters = Kratos::Parameters();

    // Build statistics records
    StatisticsRecord::Pointer p_turbulence_statistics = Kratos::make_shared<StatisticsRecord>();

    // Initialize STATISTICS_CONTAINER in ProcessInfo
    p_turbulence_statistics->InitializeStorage(mrModelPart.Elements());
    mrModelPart.GetProcessInfo().SetValue(STATISTICS_CONTAINER,p_turbulence_statistics);

    KRATOS_CATCH("");
}

/// Update statistics.
void ExecuteFinalizeSolutionStep() override
{
    if (mrModelPart.GetProcessInfo()[TIME] >= mStartTime)
    {
        auto p_turbulence_statistics = mrModelPart.GetProcessInfo().GetValue(STATISTICS_CONTAINER);
        p_turbulence_statistics->SampleIntegrationPointResults(mrModelPart);
    }
}

/// Output simulation results to file.
void ExecuteFinalize() override
{
    auto p_turbulence_statistics = mrModelPart.GetProcessInfo().GetValue(STATISTICS_CONTAINER);
    p_turbulence_statistics->PrintToFile(mrModelPart);
}

///@}
///@name Access
///@{

///@}
///@name Inquiry
///@{

///@}
///@name Input and output
///@{

/// Turn back information as a string.
std::string Info() const override
{
    std::stringstream buffer;
    buffer << "IntegrationPointStatisticsProcess";
    return buffer.str();
}

/// Print information about this object.
void PrintInfo(std::ostream &rOStream) const override { rOStream << "IntegrationPointStatisticsProcess"; }

/// Print object's data.
void PrintData(std::ostream &rOStream) const override {}

///@}
///@name Friends
///@{

///@}

protected:
///@name Protected static Member Variables
///@{

///@}
///@name Protected member Variables
///@{

///@}
///@name Protected Operators
///@{

///@}
///@name Protected Operations
///@{

StatisticsSampler::Pointer CreateAverageSampler(Kratos::Parameters Parameters) const
{
    Kratos::Parameters default_parameters(R"({
        "statistic" : "average",
        "variable": "",
        "type": "value"
    })");

    Parameters.ValidateAndAssignDefaults(default_parameters);
    std::string variable_name = Parameters["variable"].GetString();
    std::string type = Parameters["type"].GetString();
    KRATOS_ERROR_IF(type.compare("value") != 0) << "Trying to define an average statistic of unsupported type " << type << "." << std::endl;

    if (KratosComponents<Variable<double>>::Has(variable_name))
    {
        // build double variable sampler
        Variable<double> variable = KratosComponents<Variable<double>>::Get(variable_name);
        auto value_getter = Kratos::Internals::MakeSamplerAtLocalCoordinate::ValueGetter(variable);
        return Kratos::make_shared<ScalarAverageSampler>(value_getter,variable_name);
    }
    else if (KratosComponents<Variable<array_1d<double,3>>>::Has(variable_name))
    {
        // build vector sampler
        Variable<array_1d<double,3>> variable = KratosComponents<Variable<array_1d<double,3>>>::Get(variable_name);
        auto value_getter = Kratos::Internals::MakeSamplerAtLocalCoordinate::ValueGetter(variable);
        std::vector<std::string> tags;
        tags.push_back(std::string(variable_name+"_X"));
        tags.push_back(std::string(variable_name+"_Y"));
        tags.push_back(std::string(variable_name+"_Z"));
        return Kratos::make_shared<VectorAverageSampler<array_1d<double,3>>>(value_getter,3,tags);
    }
    else
    {
        KRATOS_ERROR
        << "Trying to define an average statistic for variable " << variable_name
        << " which is not a variable of a supported type." << std::endl;
    }
}
/*
StatisticsSampler::Pointer CreateVarianceSampler(Kratos::Parameters Parameters) const
{
    Kratos::Parameters default_parameters(R"({
        "statistic" : "",
        "variable": "",
        "type": "value"
    })");
}*/

///@}
///@name Protected  Access
///@{

///@}
///@name Protected Inquiry
///@{

///@}
///@name Protected LifeCycle
///@{

///@}

private:
///@name Static Member Variables
///@{

///@}
///@name Member Variables
///@{

ModelPart& mrModelPart;

Kratos::Parameters mParameters;

double mStartTime;

///@}
///@name Private Operators
///@{

///@}
///@name Private Operations
///@{

///@}
///@name Private  Access
///@{

///@}
///@name Private Inquiry
///@{

///@}
///@name Un accessible methods
///@{

/// Assignment operator.
IntegrationPointStatisticsProcess &operator=(IntegrationPointStatisticsProcess const &rOther) = delete;

/// Copy constructor.
IntegrationPointStatisticsProcess(IntegrationPointStatisticsProcess const &rOther) = delete;

///@}

}; // Class IntegrationPointStatisticsProcess

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function
inline std::istream &operator>>(std::istream &rIStream,
                                IntegrationPointStatisticsProcess &rThis)
{
    return rIStream;
}

/// output stream function
inline std::ostream &operator<<(std::ostream &rOStream,
                                const IntegrationPointStatisticsProcess &rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_INTEGRATION_POINT_STATISTICS_PROCESS_H_INCLUDED  defined
