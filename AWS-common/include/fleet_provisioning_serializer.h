/*
 * AWS IoT Device SDK for Embedded C 202211.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright 2023 Morse Micro
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * This file declares functions for serializing and parsing CBOR encoded Fleet
 * Provisioning API payloads.
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Creates the request payload to be published to the
 * @c CreateCertificateFromCsr API in order to request a certificate from AWS IoT
 * for the included Certificate Signing Request (CSR).
 *
 * @param[in] pBuffer Buffer into which to write the publish request payload.
 * @param[in] bufferLength Length of @c pBuffer.
 * @param[in] pCsr The CSR to include in the request payload.
 * @param[in] csrLength The length of @c pCsr.
 * @param[out] pOutLengthWritten The length of the publish request payload.
 * @returns True on success
 */
bool generateCsrRequest(uint8_t * pBuffer,
                        size_t bufferLength,
                        const char * pCsr,
                        size_t csrLength,
                        size_t * pOutLengthWritten);

/**
 * @brief Creates the request payload to be published to the @c RegisterThing API
 * in order to activate the provisioned certificate and receive a Thing name.
 *
 * @param[in] pBuffer Buffer into which to write the publish request payload.
 * @param[in] bufferLength Length of buffer.
 * @param[in] pCertificateOwnershipToken The certificate's certificate ownership token.
 * @param[in] certificateOwnershipTokenLength Length of @c certificateOwnershipToken.
 * @param[in] pDeviceSerial The serial number of the device, used to generate the thing name.
 * @param[in] serialLength Length of serial number.
 * @param[out] pOutLengthWritten The length of the publish request payload.
 * @returns True on success
 */
bool generateRegisterThingRequest(uint8_t * pBuffer,
                                  size_t bufferLength,
                                  const char * pCertificateOwnershipToken,
                                  size_t certificateOwnershipTokenLength,
                                  const char * pDeviceSerial,
                                  size_t serialLength,
                                  size_t * pOutLengthWritten);

/**
 * @brief Extracts the certificate, certificate ID, and certificate ownership
 * token from a @c CreateCertificateFromCsr accepted response. These are copied
 * to the provided buffers so that they can outlive the data in the response
 * buffer and as CBOR strings may be chunked.
 *
 * @param[in] pResponse The response payload.
 * @param[in] length Length of @c pResponse.
 * @param[in] pCertificateBuffer The buffer to which to write the certificate.
 * @param[in,out] pCertificateBufferLength The length of @c pCertificateBuffer.
 * The length written is output here.
 * @param[in] pCertificateIdBuffer The buffer to which to write the certificate ID.
 * @param[in,out] pCertificateIdBufferLength The length of
 * @c pCertificateIdBuffer. The length written is output here.
 * @param[in] pOwnershipTokenBuffer The buffer to which to write the
 * certificate ownership token.
 * @param[in,out] pOwnershipTokenBufferLength The length of
 * @c pOwnershipTokenBuffer. The length written is output here.
 * @returns True on success
 */
bool parseCsrResponse(const uint8_t * pResponse,
                      size_t length,
                      char * pCertificateBuffer,
                      size_t * pCertificateBufferLength,
                      char * pCertificateIdBuffer,
                      size_t * pCertificateIdBufferLength,
                      char * pOwnershipTokenBuffer,
                      size_t * pOwnershipTokenBufferLength);

/**
 * @brief Extracts the Thing name from a @c RegisterThing accepted response.
 *
 * @param[in] pResponse The response document.
 * @param[in] length Length of @c pResponse.
 * @param[in] pThingNameBuffer The buffer to which to write the Thing name.
 * @param[in,out] pThingNameBufferLength The length of @c pThingNameBuffer. The
 * written length is output here.
 * @returns True on success
 */
bool parseRegisterThingResponse(const uint8_t * pResponse,
                                size_t length,
                                char * pThingNameBuffer,
                                size_t * pThingNameBufferLength);

/**
 * @brief Converts a CBOR document into a pretty printed string.
 *
 * @param[in] cbor The CBOR document.
 * @param[in] length The length of the CBOR document.
 *
 * @returns The pretty printed string on success. "" on error.
 */
const char * getStringFromCbor(const uint8_t * cbor,
                               size_t length);
