#include "rs2g.h"
#include "global.h"

void rs2gGeodesic2Geocentric(VecB& input)
{
//#if defined(DEBUG_ARTAS_TRF)
//   printf("Geodesic2Geocentric(x:%g y:%g z:%g)", input[0], input[1], input[2]);
//#endif

   double v = EE_A / sqrt(1 - EE_E2 * pow(sin(input[0]), 2));

   double x = (v + input[2]) * cos(input[0]) * cos(input[1]);
   double y = (v + input[2]) * cos(input[0]) * sin(input[1]);
   double z = (v * (1 - EE_E2) + input[2]) * sin(input[0]);

   input[0] = x;
   input[1] = y;
   input[2] = z;

//#if defined(DEBUG_ARTAS_TRF)
//   printf(" => x:%g y:%g z:%g\n", input[0], input[1], input[2]);
//#endif
}

void rs2gFillMat(MatA& A, double lat, double lon) //, Radar& radar);
{
//    A(0,0) = -sin(radar.Long());
//    A(0,1) = cos(radar.Long());
//    A(0,2) = 0.0;
//    A(1,0) = -sin(radar.Lat()) * cos(radar.Long());
//    A(1,1) = -sin(radar.Lat()) * sin(radar.Long());
//    A(1,2) = cos(radar.Lat());
//    A(2,0) = cos(radar.Lat()) * cos(radar.Long());
//    A(2,1) = cos(radar.Lat()) * sin(radar.Long());
//    A(2,2) = sin(radar.Lat());

   A(0,0) = -sin(lon);
   A(0,1) = cos(lon);
   A(0,2) = 0.0;
   A(1,0) = -sin(lat) * cos(lon);
   A(1,1) = -sin(lat) * sin(lon);
   A(1,2) = cos(lat);
   A(2,0) = cos(lat) * cos(lon);
   A(2,1) = cos(lat) * sin(lon);
   A(2,2) = sin(lat);

//#if defined(DEBUG_ARTAS_TRF)
//   print_all_matrix(A);
//#endif
}

void rs2gFillVec(VecB& b, double lat, double lon, double height)
{
   b[0] = lat;
   b[1] = lon;
   b[2] = height;

   rs2gGeodesic2Geocentric(b);

//#if defined(DEBUG_ARTAS_TRF)
//   print_vector(b);
//#endif
}

void geodesic2Geocentric(VecB& input)
{
//#if defined(DEBUG_ARTAS_TRF)
//   printf("Geodesic2Geocentric(x:%g y:%g z:%g)", input[0], input[1], input[2]);
//#endif

   double v = EE_A / sqrt(1 - EE_E2 * pow(sin(input[0]), 2));

   double x = (v + input[2]) * cos(input[0]) * cos(input[1]);
   double y = (v + input[2]) * cos(input[0]) * sin(input[1]);
   double z = (v * (1 - EE_E2) + input[2]) * sin(input[0]);

   input[0] = x;
   input[1] = y;
   input[2] = z;

//#if defined(DEBUG_ARTAS_TRF)
//   printf(" => x:%g y:%g z:%g\n", input[0], input[1], input[2]);
//#endif
}

bool geocentric2Geodesic(VecB& input)
{
//#if defined(DEBUG_ARTAS_TRF)
//   printf("Geocentric2Geodesic(x:%g y:%g z:%g)\n", input[0], input[1], input[2]);
//#endif

   double d_xy = sqrt(pow(input[0], 2) + pow(input[1], 2));

   double G = atan(input[1]/input[0]);

   double L = atan(input[2] / (d_xy * (1 - EE_A * EE_E2 / sqrt(pow(d_xy, 2) + pow(input[2], 2)))));
   double eta = EE_A / sqrt(1 - EE_E2 * pow(sin(L), 2));
   double H = d_xy / cos(L) - eta;

   double Li;
   if (L >= 0.0)
      Li = -0.1;
   else
      Li = 0.1;

   while (fabs(L - Li) > PRECISION_GEODESIC) {
      Li = L;
      L = atan(input[2] * (1 + H / eta) / (d_xy * (1 - EE_E2 + H / eta)));
      eta = EE_A / sqrt(1 - EE_E2 * pow(sin(L), 2));
      H = d_xy / cos(L) - eta;
   }

   input[0] = L * RAD2DEG;
   input[1] = G * RAD2DEG;
   input[2] = H;

//#if defined(DEBUG_ARTAS_TRF)
//   std::cout << "Geocentric2Geodesic() => x:" << input[0] << " y:" << input[1] << " z:" << input[2] << "\n";
//#endif
   return !isnan(input[0]) && !isnan(input[1]);
}
