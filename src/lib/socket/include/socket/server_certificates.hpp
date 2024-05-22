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
                                "MIIDVDCCAjygAwIBAgIUX3cVQ47QyJp7SOy0RPzP743W9MwwDQYJKoZIhvcNAQEL\n"
                                "BQAwOzELMAkGA1UEBhMCQVUxDDAKBgNVBAgMA05TVzEPMA0GA1UEBwwGU3lkbmV5\n"
                                "MQ0wCwYDVQQKDARhc2lvMB4XDTIxMTExMTIxMTA1MloXDTI2MTExMDIxMTA1Mlow\n"
                                "OzELMAkGA1UEBhMCQVUxDDAKBgNVBAgMA05TVzEPMA0GA1UEBwwGU3lkbmV5MQ0w\n"
                                "CwYDVQQKDARhc2lvMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyURD\n"
                                "LjKxTCkapmWhY0bP1NaaOPzIJTTB0dzREOlmRmBmiHpW7DaRx7qBm6jYDKQ7OCbz\n"
                                "30/j8K4TjHOLIwxzXhXMYTJOcN2giPHNUBvm9oEuDAhYgltArJQnBBEH+3C1hCIv\n"
                                "1+uhTWo0HpGXTeJnvboTZ1YgmbOgr6lMhNiu9QmPX885DxWf6sDq8mRgCDX2x8sk\n"
                                "Ls0HuLSo88Osjx532yEhnrZgexsByhoRD3yrKHV5mWpaunk5BMsP/XMuQHayFmbO\n"
                                "siqnHJoL1znGVH003PcBGmEDmoIUqhLiBi2gWGu1pmckP9loqQUTEn0aLOVclHf4\n"
                                "slWq344zh4tJCpQMfQIDAQABo1AwTjAdBgNVHQ4EFgQUfiX1CMQrGDi9mIBAg9cg\n"
                                "m0RwLJUwHwYDVR0jBBgwFoAUfiX1CMQrGDi9mIBAg9cgm0RwLJUwDAYDVR0TBAUw\n"
                                "AwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAnDnVNSb8z/pNFaZ6YAZ+ukfNT3jjbGm1\n"
                                "10BOLqJj8s5A8/JkwjaWhky/DuGXDywgEvzXC18aNAxASeqO8h9pAZtszu6NWB4s\n"
                                "h3r+dEQakMacxrZ+jBL/cYLrUv9r3KMPKxaDnplkamqFA/9eNmoV7vDyGtGPZuD6\n"
                                "oTROtQqqDSrxthCkqnibAfusi7RmlCdvJa0kVK7krKJZAhi8W9f32+lBPv9oq3Ns\n"
                                "dAxnOj/D3UnhNoIt0EdjqUdLo2U39dt5s5Syj2rFUAgfbc02Rwx65kq8AjTRlW/M\n"
                                "KDpGsifwIB8b7wActMUO8c3GptptNVWmFm5+Mmk54P//P3tIAx9KXw==\n"
                                "-----END CERTIFICATE-----\n"
                                "-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
                                "MIIFHDBOBgkqhkiG9w0BBQ0wQTApBgkqhkiG9w0BBQwwHAQIPcVUeQ7ZElgCAggA\n"
                                "MAwGCCqGSIb3DQIJBQAwFAYIKoZIhvcNAwcECGKlFVN6/gIlBIIEyJPeknsA5dvV\n"
                                "WK27AZzs4wM6WrsD6ba+kPJyZTpon5pn4eoTiw4UCvo7C+21G9jCqbIbDrgTWHwH\n"
                                "zu6YrBFTZgRUpdLkLsyUsp4UJrZ8xJ7N/jWlG763CyluFE5oBFLz2Vt/DSDSaWdA\n"
                                "sKdxua/1kvw+KYoDqlObMFIspM5TrndcMnWkOgyvQAvnH0NZXFFBa4QGFrwWDrCo\n"
                                "m12CzMLwaPMrNFBffCTqZnL1AajT+EYqPTi+cQ5mkRpZ2cuzhdug2eki1KkD12ZN\n"
                                "3d8Ehl8bfbLEqRE/HnggleRRt7ia1xkzruqQp14JA2UsrioGMF5nvWgWrbyHfpui\n"
                                "KrCsDwDelrArW/GCki53QBQMuH8Q1YmoNrRZwlG257eA1LiJvmxRyIjKYu0RP+Q8\n"
                                "EOldycy51VsPVPApUbv9r4IJldrIJiwlmRxxM36uKtFhfojnsWzJORDYI8C37uKi\n"
                                "UEPiD4GSEH6C0lGO94IoZPjl9z/rZ0qZx1VRHl3nxZc0AQvvj9bWMbyRwsgb6wod\n"
                                "P8JSy6uEib9UxuenNHzTd48GcNhJbhOqd4IV0XKhi8W1Kil3mMdc3QAwKaLTx12/\n"
                                "1GrbMui061uyeM6Rf6Xao0PApDnUNmxcFSTsoarG0yH7Q0WZMgKTDCCGhhtZKlE6\n"
                                "x7pRsnxiFaIpoh32EVIRI+ZXh2rXBcwa2ew0aEccRXtPFvcmdjevkrHuCggc+M+Y\n"
                                "seFqTHVGWf8eS6o08w095DboD0vFpZXZMRfycTbA7BiE4NYE/uc7v5cH3Ax9l2ef\n"
                                "zG7o9idDt+/RX7OcetxDFw4eQbq7PfjvrfXS1DcRUEyJ04emh7oxlkAUUNsFtabN\n"
                                "T0ggvHxcQWkYRE5oPlkbgpwKpK4LDcApXKFwCDKPur2W5Q7KHRfDLtSvZvYarJum\n"
                                "8j2pGpEis/bdTih29ofNsX6a0Yo5Tlj+9+1c/6/Xi7XvRk/Vbgkoa3iVQ3ckdCuZ\n"
                                "vO7cRYZBBs6W/Ti3hWFzPEX9FfcnSUp9bEnH4ASnmUcp8PDBJYeaI6HqpO3XbkbF\n"
                                "l70eDNakd2cDNjQzkFpBT7HO+HtqU3xNt9K0z2gMB7iTDVFyIzh26joCR2O8tiqS\n"
                                "wUJFkoLPb74GSB7WzE+gb4jXX6n9609PUoR3f896mM34uX3CsY8lA+/0ZGpHnFDA\n"
                                "ToKdmz6WKIAw0E20nyzLuLwPZgj7HLcR7zza4raipe9QxIdyJr5O+jzGt+OjSM9b\n"
                                "K1agibRE5DChqQ+P+ikOc6nG7UNyn+lKSjGEbwuzi8F0iugMgcTc/vYO8OWDNGpd\n"
                                "D1euA5OuVPdfatFa16Fyr4MJJIfE83C4/kSj05fdoyb6pJkOjHhUppVMe+ES5kwl\n"
                                "YI8RES2XVJzUSxnWsIM613YlMgIZ9xgcuIADnO7gaKJ4RQG+9wJ853Uo4+n89K7F\n"
                                "Y6KzihuYAUkbAw1pPo1TODom7A/gB9stqRUSQlZWmIgtnJ8wPjt/we0gzPM8LQzb\n"
                                "ye4KOLjH5iquFc4MU4TtN3gvp9P5R9UFrGeMImS5eMUmBQHckDNdIAtMj7M1wUpR\n"
                                "JVrzDXWDjC21sLn95NtNMPb+FRlt/biTV3IE3ZmX0kbuCRoH7b7hhR41Zpoajwl0\n"
                                "FqYigB5gnVodGUWuBgDUqA==\n"
                                "-----END ENCRYPTED PRIVATE KEY-----\n"};

  static const std::string key{"-----BEGIN CERTIFICATE-----\n"
                               "MIIDDjCCAfYCFGGcJXlIeQHiq/qOakbISU5cihPvMA0GCSqGSIb3DQEBCwUAMDsx\n"
                               "CzAJBgNVBAYTAkFVMQwwCgYDVQQIDANOU1cxDzANBgNVBAcMBlN5ZG5leTENMAsG\n"
                               "A1UECgwEYXNpbzAeFw0yMTExMTEyMTExNDRaFw0yNjExMTAyMTExNDRaMEwxCzAJ\n"
                               "BgNVBAYTAkFVMQwwCgYDVQQIDANOU1cxDzANBgNVBAcMBlN5ZG5leTENMAsGA1UE\n"
                               "CgwEYXNpbzEPMA0GA1UECwwGc2VydmVyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A\n"
                               "MIIBCgKCAQEAu67TzZCFNTZ5c1gKwXfdTH3bP0OB5n6dqXGK6jkPb699dyM4oPtY\n"
                               "2f+OyxJspk8D5pX6HuZ+hLGAsHxfCN+A3RegVZJCR/gXrrMPl/6PfDxtkeFXWy+6\n"
                               "NAPxqppMj8gbX/czTdNFwgW4J8/RLyH53utjAGQtCRr97EbBOsTSeOmyAxbw4mRt\n"
                               "EvHjacKFcSZzJDlyxCia37791MulZ4lwOZZlke5NzoOqkBIAusQgg7fMWDvH27PD\n"
                               "T+taIyBsYTVJWHf5kMklAlWYXZvNjZp/QGt13lDORg2aSc/P2pE4iLyhbZcpuW+W\n"
                               "nHB7IY1wjDL55/ORCjz3VlNIAMXHP5YQxwIDAQABMA0GCSqGSIb3DQEBCwUAA4IB\n"
                               "AQC3iiwQTgDP4ixulisYIGlJb8iuhxH9G41hm8dF/1qlIfXWxsbjb+g7M5U/3JCb\n"
                               "fwCGoz7uBdj/CKkIb5wx83GVC1yVLbNSvCIS827qxic4GACEYGGAy4Ub5tUCQrLr\n"
                               "JUQTP1WVJcgiKvk7OAcXdA+k/CcNXH153gUPDZz+/BedXe2VbM6LDsqPeSxXfbZN\n"
                               "AWXARAuly7cQk/xkYkPldRRhx4htNCbnkG1qyEqNcYQN1SY9Qhbrm2HkAUntzQ7G\n"
                               "umo4JdbIX6MJ5fYNs+yN/lJB7PiJP8Mz1AtOz4x9JxcLyuB6sapT8Tj53n9g/+m1\n"
                               "Tqd8lHpWe6oM0sgdWfzdBNir\n"
                               "-----END CERTIFICATE-----\n"
                               "-----BEGIN RSA PRIVATE KEY-----\n"
                               "Proc-Type: 4,ENCRYPTED\n"
                               "DEK-Info: DES-EDE3-CBC,9E0E6B9970E055F1\n"
                               "\n"
                               "oFvN3t50B2TSeNsRo1vwhVYlMmt0w3MD3Ar75fti4fRhBrWoH4rBPrSouWK4OMDK\n"
                               "CakfVhzJIEvaMav843grS4V+ooAsfnAtiQiQj+5xjZ8PlCuWir5gyjgqeN4zN6ZT\n"
                               "2s/oEy5Kx7Ru5AWMkv+NPW5l3iS3c+Hupfb/auITwECSCCsCBxgNtJ8z0iXjv9Dr\n"
                               "hqvqcI+4F5DSiol3QhitbpQ/k2PKu92AZhPcr9e5HrqeUD7aqfX2q3i+tnJvxC93\n"
                               "0mvy133bX2ZAGAKMheW/GuzcETfwU80wvmrwHXI9rnuAUaJhrDiIZQ79gubZ0Pcm\n"
                               "w4Y6Z5xxeErb7zKBi+pHKst8MJ7rfSz/N878su6cpESRC/w1paVX7m8/q61kzBkj\n"
                               "+h2qe9DupXBejQkriN+Z858nFtBKbWlFsmAg1FLZonworTUfV2c7S9yzMV/29VOE\n"
                               "T0Bt8BU1Hp+bCjreF4JGbnUJnRQoASpisfUkrD6Ar4GI56Edkkx6hkF0cXdcvfCN\n"
                               "qyhxzcvf/aV1M2kcpBRX9mIfjoFMFNz7uRIxyqPLaDkigwJemoaBo3n9/W5F7N7B\n"
                               "AGNEZ3QhUlc8EyWpGRZRzMevu5wdz1b2+MqItOhPXUo2XcLEj2iFkVDdbj/d9jQH\n"
                               "9ZIe4hQTSf784tAzt2Jp2RHmm22390GB1jdzG/CzbGlAKmaTTEe4FJcd/eLOEZuD\n"
                               "4+iFsEb/bWcCyXkBKkdEoDDMJW8xi5d/1ezJM03nqhvkekDqyhJUJPCbmY1SCX1X\n"
                               "4AO40jJmWQxwj26Pnnncacz4tuecZNqovz+POpKQSIctrQSV4wgf35137KBt7/O7\n"
                               "EU0N7Qj4xTigyGkEHCZt7BgD8fFrxiP1OkUmFRHEE7Dxiagz6omJV8ien+p9wIgS\n"
                               "QcqkJlgKMwVepjuCuEyvg5oIkAPvp24yhePSJkEvvoDLvM2Kub/CM+UkN8dSgsZk\n"
                               "+ZBE9Oi62AWWow8/ib8Um5DNIEZsq+5CQLeBG7f5kTIBAoA9UceaEgbxzOVUCBcX\n"
                               "L7QsJ3pZ/ei1qx/9tChO/OXa9tleXcjGg7/q/6i6pUyZuqZlt+hk+3HtFmTXIjsM\n"
                               "chbUSMEJ2isv9MDamTtunt9N92A6OxUkAs68kCO/bzotldXMKZVjOohNnML5q/3U\n"
                               "vpsuN8NNwaNHCAM7pjols4V7ebX1lDZFR1hbj+szKMhVKzFfig1lRHafgRrR6H6P\n"
                               "mUK16lKzUbmvwv2IU/3wck+4UNeB/+lRmfAfBasQytnbt+5fwwNkVSrDY2Kxe0nz\n"
                               "2k3eIYST+ygxGuNrW9rnFbBpTJb0OE5cpgun8xxDE91AmL550AcZff+9ulDBhZg1\n"
                               "eMoKUALlEdDNPffVTH8Iupn0jMnJsjXynztcEVhx4h2aA5X6/2i035uEybCn33qP\n"
                               "J7g30oe2C69BSC0YmkT3MjF6Wrhy+M8dU0dDggRhY6JSS8wESvwGenaPi7SZlbxb\n"
                               "Elmc44bHros1YfGl/xDdhju134/aMBG1GlZlReI+Rwnp9DCp04xiiJb9YKyUNaGY\n"
                               "7BTGWyie/WYYKbX1ZlxqWA9jxow8PM1XYHuanau1/aYT16L7E83HG3qkyuFIlLJh\n"
                               "-----END RSA PRIVATE KEY-----\n"
                               "-----BEGIN CERTIFICATE-----\n"
                               "MIIDVDCCAjygAwIBAgIUX3cVQ47QyJp7SOy0RPzP743W9MwwDQYJKoZIhvcNAQEL\n"
                               "BQAwOzELMAkGA1UEBhMCQVUxDDAKBgNVBAgMA05TVzEPMA0GA1UEBwwGU3lkbmV5\n"
                               "MQ0wCwYDVQQKDARhc2lvMB4XDTIxMTExMTIxMTA1MloXDTI2MTExMDIxMTA1Mlow\n"
                               "OzELMAkGA1UEBhMCQVUxDDAKBgNVBAgMA05TVzEPMA0GA1UEBwwGU3lkbmV5MQ0w\n"
                               "CwYDVQQKDARhc2lvMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyURD\n"
                               "LjKxTCkapmWhY0bP1NaaOPzIJTTB0dzREOlmRmBmiHpW7DaRx7qBm6jYDKQ7OCbz\n"
                               "30/j8K4TjHOLIwxzXhXMYTJOcN2giPHNUBvm9oEuDAhYgltArJQnBBEH+3C1hCIv\n"
                               "1+uhTWo0HpGXTeJnvboTZ1YgmbOgr6lMhNiu9QmPX885DxWf6sDq8mRgCDX2x8sk\n"
                               "Ls0HuLSo88Osjx532yEhnrZgexsByhoRD3yrKHV5mWpaunk5BMsP/XMuQHayFmbO\n"
                               "siqnHJoL1znGVH003PcBGmEDmoIUqhLiBi2gWGu1pmckP9loqQUTEn0aLOVclHf4\n"
                               "slWq344zh4tJCpQMfQIDAQABo1AwTjAdBgNVHQ4EFgQUfiX1CMQrGDi9mIBAg9cg\n"
                               "m0RwLJUwHwYDVR0jBBgwFoAUfiX1CMQrGDi9mIBAg9cgm0RwLJUwDAYDVR0TBAUw\n"
                               "AwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAnDnVNSb8z/pNFaZ6YAZ+ukfNT3jjbGm1\n"
                               "10BOLqJj8s5A8/JkwjaWhky/DuGXDywgEvzXC18aNAxASeqO8h9pAZtszu6NWB4s\n"
                               "h3r+dEQakMacxrZ+jBL/cYLrUv9r3KMPKxaDnplkamqFA/9eNmoV7vDyGtGPZuD6\n"
                               "oTROtQqqDSrxthCkqnibAfusi7RmlCdvJa0kVK7krKJZAhi8W9f32+lBPv9oq3Ns\n"
                               "dAxnOj/D3UnhNoIt0EdjqUdLo2U39dt5s5Syj2rFUAgfbc02Rwx65kq8AjTRlW/M\n"
                               "KDpGsifwIB8b7wActMUO8c3GptptNVWmFm5+Mmk54P//P3tIAx9KXw==\n"
                               "-----END CERTIFICATE-----\n"
                               "-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
                               "MIIFHDBOBgkqhkiG9w0BBQ0wQTApBgkqhkiG9w0BBQwwHAQIPcVUeQ7ZElgCAggA\n"
                               "MAwGCCqGSIb3DQIJBQAwFAYIKoZIhvcNAwcECGKlFVN6/gIlBIIEyJPeknsA5dvV\n"
                               "WK27AZzs4wM6WrsD6ba+kPJyZTpon5pn4eoTiw4UCvo7C+21G9jCqbIbDrgTWHwH\n"
                               "zu6YrBFTZgRUpdLkLsyUsp4UJrZ8xJ7N/jWlG763CyluFE5oBFLz2Vt/DSDSaWdA\n"
                               "sKdxua/1kvw+KYoDqlObMFIspM5TrndcMnWkOgyvQAvnH0NZXFFBa4QGFrwWDrCo\n"
                               "m12CzMLwaPMrNFBffCTqZnL1AajT+EYqPTi+cQ5mkRpZ2cuzhdug2eki1KkD12ZN\n"
                               "3d8Ehl8bfbLEqRE/HnggleRRt7ia1xkzruqQp14JA2UsrioGMF5nvWgWrbyHfpui\n"
                               "KrCsDwDelrArW/GCki53QBQMuH8Q1YmoNrRZwlG257eA1LiJvmxRyIjKYu0RP+Q8\n"
                               "EOldycy51VsPVPApUbv9r4IJldrIJiwlmRxxM36uKtFhfojnsWzJORDYI8C37uKi\n"
                               "UEPiD4GSEH6C0lGO94IoZPjl9z/rZ0qZx1VRHl3nxZc0AQvvj9bWMbyRwsgb6wod\n"
                               "P8JSy6uEib9UxuenNHzTd48GcNhJbhOqd4IV0XKhi8W1Kil3mMdc3QAwKaLTx12/\n"
                               "1GrbMui061uyeM6Rf6Xao0PApDnUNmxcFSTsoarG0yH7Q0WZMgKTDCCGhhtZKlE6\n"
                               "x7pRsnxiFaIpoh32EVIRI+ZXh2rXBcwa2ew0aEccRXtPFvcmdjevkrHuCggc+M+Y\n"
                               "seFqTHVGWf8eS6o08w095DboD0vFpZXZMRfycTbA7BiE4NYE/uc7v5cH3Ax9l2ef\n"
                               "zG7o9idDt+/RX7OcetxDFw4eQbq7PfjvrfXS1DcRUEyJ04emh7oxlkAUUNsFtabN\n"
                               "T0ggvHxcQWkYRE5oPlkbgpwKpK4LDcApXKFwCDKPur2W5Q7KHRfDLtSvZvYarJum\n"
                               "8j2pGpEis/bdTih29ofNsX6a0Yo5Tlj+9+1c/6/Xi7XvRk/Vbgkoa3iVQ3ckdCuZ\n"
                               "vO7cRYZBBs6W/Ti3hWFzPEX9FfcnSUp9bEnH4ASnmUcp8PDBJYeaI6HqpO3XbkbF\n"
                               "l70eDNakd2cDNjQzkFpBT7HO+HtqU3xNt9K0z2gMB7iTDVFyIzh26joCR2O8tiqS\n"
                               "wUJFkoLPb74GSB7WzE+gb4jXX6n9609PUoR3f896mM34uX3CsY8lA+/0ZGpHnFDA\n"
                               "ToKdmz6WKIAw0E20nyzLuLwPZgj7HLcR7zza4raipe9QxIdyJr5O+jzGt+OjSM9b\n"
                               "K1agibRE5DChqQ+P+ikOc6nG7UNyn+lKSjGEbwuzi8F0iugMgcTc/vYO8OWDNGpd\n"
                               "D1euA5OuVPdfatFa16Fyr4MJJIfE83C4/kSj05fdoyb6pJkOjHhUppVMe+ES5kwl\n"
                               "YI8RES2XVJzUSxnWsIM613YlMgIZ9xgcuIADnO7gaKJ4RQG+9wJ853Uo4+n89K7F\n"
                               "Y6KzihuYAUkbAw1pPo1TODom7A/gB9stqRUSQlZWmIgtnJ8wPjt/we0gzPM8LQzb\n"
                               "ye4KOLjH5iquFc4MU4TtN3gvp9P5R9UFrGeMImS5eMUmBQHckDNdIAtMj7M1wUpR\n"
                               "JVrzDXWDjC21sLn95NtNMPb+FRlt/biTV3IE3ZmX0kbuCRoH7b7hhR41Zpoajwl0\n"
                               "FqYigB5gnVodGUWuBgDUqA==\n"
                               "-----END ENCRYPTED PRIVATE KEY-----\n"};

  static const std::string dh{"-----BEGIN X9.42 DH PARAMETERS-----\n"
                              "MIIELQKCAgEA8eEBgatlTk0QKIwUqHsteHJXzEr0xAW7RLeNid8DdGjTnFw68eV+\n"
                              "s1YbipEqQVTFG+VZPaof9v0CH2KOdbgWD0KRPC49HB8O/wARFKpy5eJltVe1r07w\n"
                              "+V1Vjciqkh/vGq/yFQsbqwyWJy6AelsbHdSC9Bg9pmu4+pbVGW15zK7PkfjfoO9V\n"
                              "JYbhdqFIPg+9NRRWsl/U9UoyeeIGImI9I4k3fqtv311C6VVErFZylCj7nn45L7IY\n"
                              "GfPcCO5E8FD6EqBv5WnhMNYHMJu9brRvOoAyAyWprqR+aVzHd+DFamDQdsuQjKTk\n"
                              "3+r07znWJfjp6FVTzR5SvjrKg1knK5TibIksoKOlsFZ06bGkFWHjqXNAKhnSpCiU\n"
                              "GJd0qe7pTD+TGQJGWskJpoph+tkICUa+6MyzP9+U6T9hISIY/BpVuWn/iwdLqsvV\n"
                              "s5bg2RjRraww/4Rjm0NyEOTk2MxaLB9gcyu6zcrhX5XRiMgt+jhKwEMwJCEJYkxn\n"
                              "C0yv08A5v6RceFm1jY+8FT0IzRqsWIGV7beNdqtE0B4LBTuN1yQ6x8CKEb6mbIHG\n"
                              "d0XLGppELhD8QUwAr5HiVGTAsS9JlMGgXVkyDtxrVSeS1zHWt77skRQ9UlfJfYMm\n"
                              "5wxHj5QCdQ7ma3oMMZBZutNlkevMzYHIxwqhZstf4ud0j0VMftwZ3GMCggIBAJL7\n"
                              "cU6/YSxzYDGUcPc6WGT+FAghR0LdhT3Q3Xz1rQRBwSSMPm9NZhyJC3fb+hBl/44X\n"
                              "AHOu1/3dYIWlXPMuHZZdtlt+hkD/sIob16By20Z0p81Kvh3HQglVDJGlFzTmQbTd\n"
                              "CM/EP8eJFiNeMcJB0RgeKyzikRsV4r/acZZM56swTlsRvSDFMh3v9YyYTZbCTwxm\n"
                              "PIVOsxI8uBoWRCzY3yWzjmlZ2u5TR+Z0IVonmNqk6XAKRgxV12YfQVXzzpPZQA9S\n"
                              "GHoD/NQoMdBAi3p4pLEYJ5qMRLYPQbBfSNu5eP9e5uZOiBp/28bVnzEelDGuFupr\n"
                              "lvywkZEVL1KaYtZ/dGkPFXt+A7mNsw5xeDTRjgL4uUJvCNIkjgVEfIrLdUf5IVsU\n"
                              "V8BgunAvgfH+Hy6Tx89v/QGsUWJPhjkSZnXwJ7Ipcsm+Zg0zAZwKDZEQtDLXVPRL\n"
                              "OfCfXMXvlw2u6OHlhSbqO69zZG3dJTlcnjy06n4JYcUim7Mj3Kduige6VWofeEk9\n"
                              "M/kHrPqNdFE5tHLh2nHcw9yExKHN+l/OUGAvNDfOVS+S+fNSKT0RfC9dMpHLN+Ok\n"
                              "fGuskGzLQAVYF+DYEcnJ++0LEFtd0J381ndSc0iJuH60rKDTqxoRqv7lcdwLFlEV\n"
                              "sQSRoas8s+y+b3FfSu7j0p44qLG9oxZ/WFcWnuUyAiEAjvAD9p3bw8AH0s7dUrX/\n"
                              "RT1Ge+J/n0tlsgw+LY3zafE=\n"
                              "-----END X9.42 DH PARAMETERS-----\n"};

  ctx.set_options(boost::asio::ssl::context::default_workarounds |
                  boost::asio::ssl::context::single_dh_use   //|                   boost::asio::ssl::context::no_sslv2 | //
                                                             //                  boost::asio::ssl::context::no_tlsv1_2
  );

  ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose) {
    // return "l23k4019u 34tu2 984y52345";
    return "test";
  });
  ctx.use_certificate_chain(boost::asio::const_buffer(cert.data(), cert.size()));
  ctx.use_private_key(boost::asio::const_buffer(cert.data(), cert.size()), boost::asio::ssl::context::file_format::pem);
  // ctx.use_private_key(boost::asio::const_buffer(key.data(), key.size()), boost::asio::ssl::context::file_format::pem);
  ctx.use_tmp_dh(boost::asio::const_buffer(dh.data(), dh.size()));
}
