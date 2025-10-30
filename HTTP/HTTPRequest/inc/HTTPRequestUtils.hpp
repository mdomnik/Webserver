/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestUtils.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 13:14:42 by mdomnik           #+#    #+#             */
/*   Updated: 2025/10/30 13:14:54 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTUTILS_HPP
#define HTTPREQUESTUTILS_HPP

std::string TrimHTTPLine(const std::string& str);
std::vector<std::string> HTTPSplitTokens(const std::string &str);
int HttpStringToSizet(const std::string &str, size_t &value);
int HTTPValidateMethod(const std::string &method);
int HTTPValidatePath(const std::string &path);
int HTTPValidateVersion(const std::string &version);

#endif