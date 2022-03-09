
add_library( planck_wavelength SHARED "planck_wavelength.cpp" )
target_link_libraries( planck_wavelength GSL::gsl )
