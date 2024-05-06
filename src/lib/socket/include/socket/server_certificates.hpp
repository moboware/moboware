#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <cstddef>
#include <memory>

/*  Load a signed certificate into the ssl context, and configure the context for use with a server.

    For this to work with the browser or operating system, it is necessary to import the "Beast Test CA" certificate into
    the local certificate store, browser, or operating system depending on your environment Please see the documentation
    accompanying the Beast certificate for more details.
*/
inline void LoadServerCertificate(boost::asio::ssl::context &ctx)
{
  /*
      The certificate was generated from bash on Ubuntu (OpenSSL 1.1.1f) using:

      - openssl dhparam -out dh.pem 2048
      - openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 25000 -out cert.pem -subj
     "/C=NL/ST=Amsterdam/L=Centrum/O=Moboware/CN=moboware.com"
      - copy the generated certificates of the dh.pem, cert.pem and key.pem file into this header file
  */

  static const std::string cert{"-----BEGIN CERTIFICATE-----\n"
                                "MIIDnTCCAoWgAwIBAgIUGioWKPBylXfwfu/UlQGTAaVGxyswDQYJKoZIhvcNAQEL\n"
                                "BQAwXTELMAkGA1UEBhMCTkwxEjAQBgNVBAgMCUFtc3RlcmRhbTEQMA4GA1UEBwwH\n"
                                "Q2VudHJ1bTERMA8GA1UECgwITW9ib3dhcmUxFTATBgNVBAMMDG1vYm93YXJlLmNv\n"
                                "bTAgFw0yNDA1MDExNDI0MDJaGA8yMDkyMTAxMTE0MjQwMlowXTELMAkGA1UEBhMC\n"
                                "TkwxEjAQBgNVBAgMCUFtc3RlcmRhbTEQMA4GA1UEBwwHQ2VudHJ1bTERMA8GA1UE\n"
                                "CgwITW9ib3dhcmUxFTATBgNVBAMMDG1vYm93YXJlLmNvbTCCASIwDQYJKoZIhvcN\n"
                                "AQEBBQADggEPADCCAQoCggEBAMhvZW+pDsJVbU/LVECKdOdl0AVCnI3hlDXg1kpy\n"
                                "kUWKxldBlCFZ4RuO9oVcp1oznNS2Z4uq7WuMs7jZY9CUFbjYLBvBzQz09HZAf0Yh\n"
                                "OkdKrYCDglle8nJt0m9eNbtnENdYWFE4bG6aiqlLRXt1U0bXZQfJqwQz64pl1ROw\n"
                                "aAWrTrmO2W7Rdufjd4wij6Xd8A1IFcaRsGJEKskgRJKMsGeJ1H8zARGVFDdrJufW\n"
                                "AEGciSnBdx1GjN7VQfEDOk/Gokeq/b1UF1MV4mRQ9p0cVp4VVL2cl+mwdkMFJjRo\n"
                                "OXYcGg4QvjOgrIQwmtn5/PtsJd8ikfmOmXDwAehqhrQ5jkkCAwEAAaNTMFEwHQYD\n"
                                "VR0OBBYEFMfeCsLevYhlQ1BR25s29Rt6yQK5MB8GA1UdIwQYMBaAFMfeCsLevYhl\n"
                                "Q1BR25s29Rt6yQK5MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEB\n"
                                "AFaikJs9Agi3URzsZs6PiJNd81XKyAIFbb5/zVMHA2GitKl54z8jYeYxH7MDpvJb\n"
                                "FJPebumFZDIrUXxH1F8dgbX8Gd9hMa0Opt6TbnLPrCFN0O/wehQG25rSiICDI8Br\n"
                                "edeNTCZyQ7dbxinIxqkJBs3jHygluA3rLz/CQnPVjRqnMnIEGiOisUMC/SFVYKBG\n"
                                "1pcwWigjaDDGPsq3QYDOjaYPAOM7oIgy0w2LpLJCBTE1J667tUNGH+R63s/z+WMW\n"
                                "iXPOoAn53lgUIbnqaFJp5Jdn3qv8kJnk64xYByKHRnqXVDLaFmSzrn6lfZWKPefh\n"
                                "yMfirdQ35oD+9vkLnkzNQNA=\n"
                                "-----END CERTIFICATE-----\n"};

  static const std::string key{"-----BEGIN PRIVATE KEY-----\n"
                               "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDIb2VvqQ7CVW1P\n"
                               "y1RAinTnZdAFQpyN4ZQ14NZKcpFFisZXQZQhWeEbjvaFXKdaM5zUtmeLqu1rjLO4\n"
                               "2WPQlBW42Cwbwc0M9PR2QH9GITpHSq2Ag4JZXvJybdJvXjW7ZxDXWFhROGxumoqp\n"
                               "S0V7dVNG12UHyasEM+uKZdUTsGgFq065jtlu0Xbn43eMIo+l3fANSBXGkbBiRCrJ\n"
                               "IESSjLBnidR/MwERlRQ3aybn1gBBnIkpwXcdRoze1UHxAzpPxqJHqv29VBdTFeJk\n"
                               "UPadHFaeFVS9nJfpsHZDBSY0aDl2HBoOEL4zoKyEMJrZ+fz7bCXfIpH5jplw8AHo\n"
                               "aoa0OY5JAgMBAAECggEAIb9T0m2iDQlqKs2+5VEKpu1nDYOuLVtmY330Rrushwt7\n"
                               "IrTpdoYWF1hGXKUBjnfzN4NwwrzoVbvuhLihjDxqveEz6AaE+mICCSkJ5OpyUA+c\n"
                               "T++UkmygoLCqBlNT87ZCSSfYUvCPnT3LC/8e908FUv+wRJPphf8h8P79Vd/VW+K0\n"
                               "SKHHuy7i70pFkodAUN7uWsIfZvrqsvn+Oh7MxV7IDX0pJoPEgnTUPQy5PF+VRv6W\n"
                               "b7kIZGmO5q5fMa89HHz+WrXXrY4L1GwL+Cotb8wZ2CFbpi99UcmRAvVlXXGbF08q\n"
                               "LdjkClWsSEBRLJg1J54qdgykWZj+/BTYWINmItA+qQKBgQDlqHR4NyHuY70CcGNy\n"
                               "mLdoZsACCF6uL4tCkIl6ni5mIXusdiZYfAliGukLAprS7zVJkLwNbRmBQO/E7ZFv\n"
                               "Se/eZn7LI1JcREhayIy1J4vhSE5LIoYsI4jZ4l7Y5i8uPiYcWOrvKt87g2y5Lxpo\n"
                               "ZrBsDc2EkR+q7uZULiFcN78vLQKBgQDfbNt/2prNJjJ6bQItNRJpCKUM1q3t73kF\n"
                               "c0vASa9aQ4WEWAwW7fnhI/fIvpiHuvf7pkHnceKc1NbXJ9XQFjQ90FkdidTIj5mu\n"
                               "KXIFZSZB2Vb8n9oZ2TpNlwJgCBoDWBwwp4oO8eMaExFQKMs9ZkqICYUo6uPqY18U\n"
                               "UjpxLuZtDQKBgB4EdKS54A9+rdfBxN3XpDFbLMGXQnjV8gfTSITkZAApZDbYgo6S\n"
                               "N8/A4LD1uDLV3UsuRcYjem/wguZkftvi+B1DnFME3OD+c86Z4/pRnvDlOIaNjqf6\n"
                               "RQrlkawb+hK7QNP3s01rdx4aQX268GKw+WJGM5SeO1tmOmjKVsXs62pJAoGBALI0\n"
                               "0IsVIR36lo6hvfey+iE+3O71mLtMo/nd8ZDI9Q2JB8j8tX/ghsu19aBvrZetYLie\n"
                               "7rgdVUQEW+w3AMq9uyOGuF3gHxFtwIVy11hISpCBO2m6UqsFYgSbKzDPpXf/YlTI\n"
                               "fUzYUGzuwoNuVlV97nq9JTHy0KGWazL5N1Yps29NAoGAasPtaxfIkXMlUYqjml+q\n"
                               "BaDcjv3TG4CHyPdTDYiTGOPhLTDFArQLY3LTVM2i7A0p9jiXEjqwQ70OQturHLeN\n"
                               "KCpz9jBlEUFiuDZO2h2ogDGdUn5KdFJ8qBx7Nejm4yEhzrOtdiLvTh7WziHxwL4a\n"
                               "61xewOdtQaUW3qLAW7Ni94E=\n"
                               "-----END PRIVATE KEY-----\n"};

  static const std::string dh{"-----BEGIN DH PARAMETERS-----\n"
                              "MIIBCAKCAQEA7BlYbaL4+lH7yZTVgySirczHAy3+WKEoErYVT8f3WNZiWW621SXv\n"
                              "ClFPxUftZKxYO/M3W3fqkUYdxC67IAHY9O2ocLRwvVZ4MtLRYGco71XIvL3p/A8v\n"
                              "085WQ6SRSQNK165jcAkZLCF/Nlt7cErXkxetAaxxjxKKdoeZUePwssAEc5x3yKhd\n"
                              "p9ZpsyGCucpqHbylwBxAtYt5im6R9DVrSav8OzaqAcR5S059SxtHG2s/5HhWs/xz\n"
                              "5NM3oibzJZVB/3VLAtcoQx4ukCmyDodjTN9kPalPhVANX3ACl6hWkdR75KjmRAS7\n"
                              "5Ha+KTp7hfdlFomi1e2nBlGMLmdwqkdvLwIBAg==\n"
                              "-----END DH PARAMETERS-----\n"};

  ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose) {
    return "l23k4019u 34tu2 984y52345";
  });

  ctx.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::tlsv13 |
                  boost::asio::ssl::context::tlsv13_client | boost::asio::ssl::context::tlsv13_server |
                  boost::asio::ssl::context::single_dh_use);

  ctx.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));
  ctx.use_private_key(boost::asio::buffer(key.data(), key.size()), boost::asio::ssl::context::file_format::pem);
  ctx.use_tmp_dh(boost::asio::buffer(dh.data(), dh.size()));
}
