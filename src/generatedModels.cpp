#include "generatedModels.h"
#include "heatZone.h"

#define MODELDECL HEAT_ZONE_MODEL static
#include "_generated_model_regression.inl"
#include "_generated_model_classification.inl"

HEAT_ZONE_MODEL bool predictModel(const double *a)
{
    return regression(a) > 0 && classification(a) > 0;
}

HEAT_ZONE_MODEL double predictModelRegression(const double *a)
{
    return regression(a);
}

HEAT_ZONE_MODEL double predictModelClassification(const double *a)
{
    return classification(a);
}
