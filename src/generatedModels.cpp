#include "generatedModels.h"
#include "heatZone.h"

#define MODELDECL HEAT_ZONE_MODEL static
#include "_generated_model_regression.inl"
#include "_generated_model_classification.inl"

HEAT_ZONE_MODEL bool predictModel(const float *a)
{
    return regression(a) > 0 && classification(a) > 0;
}

HEAT_ZONE_MODEL float predictModelRegression(const float *a)
{
    return regression(a);
}

HEAT_ZONE_MODEL float predictModelClassification(const float *a)
{
    return classification(a);
}
